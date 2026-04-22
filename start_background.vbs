Set oShell = CreateObject("WScript.Shell")
oShell.Run "cmd.exe /c cd host && python bridge.py", 0, False
