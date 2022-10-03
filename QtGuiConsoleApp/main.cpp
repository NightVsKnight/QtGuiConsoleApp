#include <QApplication>
#include <QMessageBox>

#ifdef Q_OS_WIN

// Solution posted to https://stackoverflow.com/a/73942013/252308

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

QString consolePromptClear()
{
    QString prompt = nullptr;

//    auto isDebuggerPresent = IsDebuggerPresent();
//    qDebug() << "isDebuggerPresent=" << isDebuggerPresent;

    auto bSuccess = AttachConsole(ATTACH_PARENT_PROCESS);
//    qDebug() << "bResult=" << bResult;
    if (bSuccess)
    {
        auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
//        qDebug() << "hStdOut=" << hStdOut;
        if (hStdOut != INVALID_HANDLE_VALUE)
        {
#if 0
            {
                // Fake a prompt for debugging when launched from IDE terminal
                //                                                                                                     1
                //           1         2         3         4         5         6         7         8         9         0         1
                // 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
                //"C:\Dev\GitHub\NightVsKnight\QtConsoleApp\build-QtConsoleApp-Desktop_Qt_6_3_2_MSVC2019_64bit-Debug\debug>"
                auto dwBufferLength = GetCurrentDirectory(0, 0); // length without NULL terminator
                auto lpFakePrompt = (LPWSTR)LocalAlloc(LPTR, dwBufferLength * sizeof(WCHAR));
                GetCurrentDirectory(dwBufferLength, lpFakePrompt);
                lpFakePrompt[dwBufferLength-1] = L'>';
                DWORD dwNumberOfChars;
                WriteConsoleW(hStdOut, L"\n", 1, &dwNumberOfChars, 0);
                WriteConsoleW(hStdOut, lpFakePrompt, dwBufferLength, &dwNumberOfChars, 0);
                WriteConsoleW(hStdOut, L"\n", 1, &dwNumberOfChars, 0);
                WriteConsoleW(hStdOut, L"\n", 1, &dwNumberOfChars, 0);
                WriteConsoleW(hStdOut, lpFakePrompt, dwBufferLength, &dwNumberOfChars, 0);
                LocalFree(lpFakePrompt);
            }
#endif
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            bSuccess = GetConsoleScreenBufferInfo(hStdOut, &csbi);
            if (bSuccess)
            {
                auto dwConsoleColumnWidth = (DWORD)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
                auto xEnd = csbi.dwCursorPosition.X;
                auto yEnd = csbi.dwCursorPosition.Y;

                if (xEnd != 0 || yEnd != 0)
                {
                    DWORD dwNumberOfChars;

                    SHORT yBegin = yEnd;
                    {
                        // Walk backwards to find first all blank line
                        auto pBuffer = (LPWSTR)LocalAlloc(LPTR, dwConsoleColumnWidth * sizeof(WCHAR));
                        while (yBegin)
                        {
                            COORD dwReadCoord = { 0, yBegin };
                            bSuccess = ReadConsoleOutputCharacterW(hStdOut, pBuffer, dwConsoleColumnWidth, dwReadCoord, &dwNumberOfChars);
                            if (!bSuccess) break;

                            DWORD i;
                            for (i=0; i < dwNumberOfChars; ++i)
                            {
                                WCHAR wchar = pBuffer[i];
                                if (wchar != L' ')
                                {
                                    --yBegin;
                                    break;
                                }
                            }

                            if (i == dwNumberOfChars)
                            {
                                // Found all blank line; we want the *next* [non-blank] line
                                yBegin++;
                                break;
                            }
                        }
                        LocalFree(pBuffer);
                    }

                    auto promptLength = (yEnd - yBegin) * dwConsoleColumnWidth + xEnd;
                    auto lpPromptBuffer = (LPWSTR)LocalAlloc(LPTR, promptLength * sizeof(WCHAR));
                    COORD dwPromptCoord = { 0, yBegin };
                    bSuccess = ReadConsoleOutputCharacterW(hStdOut, lpPromptBuffer, promptLength, dwPromptCoord, &dwNumberOfChars);
                    if (bSuccess)
                    {
                        Q_ASSERT(promptLength == dwNumberOfChars);

                        prompt = QString::fromWCharArray(lpPromptBuffer, dwNumberOfChars);

                        bSuccess = SetConsoleCursorPosition(hStdOut, dwPromptCoord);
                        if (bSuccess)
                        {
                            FillConsoleOutputCharacterW(hStdOut, L' ', promptLength, dwPromptCoord, &dwNumberOfChars);
                        }
                    }
                    LocalFree(lpPromptBuffer);
                }
            }
        }
    }

    if (prompt.isEmpty())
    {
        FreeConsole();
        return nullptr;
    }
    else
    {
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        return prompt;
    }
}

void consolePromptRestore(const QString& prompt)
{
    if (prompt.isEmpty()) return;

    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    BOOL bSuccess = GetConsoleScreenBufferInfo(hStdOut, &csbi);
    if (!bSuccess) return;

    auto xEnd = csbi.dwCursorPosition.X;
    auto yEnd = csbi.dwCursorPosition.Y;

    if (xEnd == 0 && yEnd == 0) return;

    auto buffer = prompt.toStdWString();
    auto lpBuffer = buffer.data();
    auto nLength = (DWORD)buffer.length();
    COORD dwWriteCoord = { 0, (SHORT)(yEnd + 1) };
    DWORD dwNumberOfCharsWritten;
    WriteConsoleOutputCharacterW(hStdOut, lpBuffer, nLength, dwWriteCoord, &dwNumberOfCharsWritten);

    dwWriteCoord = { (SHORT)dwNumberOfCharsWritten, (SHORT)(yEnd + 1) };
    SetConsoleCursorPosition(hStdOut, dwWriteCoord);
}

#else

// Non-Windows impl...

#endif

int main(int argc, char *argv[])
{
    // NOTE: Any console output before call to consolePromptClear() may get cleared.
    // NOTE: Console vs GUI mode has **NOTHING** to do with being passed arguments; You can easily pass arguments to GUI apps.

    int returnCode;

    auto prompt = consolePromptClear();
    if (prompt.isEmpty())
    {
        QApplication a(argc, argv);
        a.setQuitOnLastWindowClosed(true);

        QMessageBox msgBox(nullptr);
        msgBox.setWindowTitle(a.applicationName());
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("App is detected to be running as a GUI");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.show();

        returnCode = a.exec();
    }
    else
    {
        QCoreApplication a(argc, argv);
        QTextStream qout(stdout);
        qout << "App is detected to be running as a Console" << Qt::endl;

        returnCode = 0;

        consolePromptRestore(prompt);
    }
    return returnCode;
}
