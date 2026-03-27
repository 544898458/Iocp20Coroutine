$filePath = "d:\比较\Iocp20Coroutine\Iocp20Coroutine\GateSvr\GateSvrMain.cpp"
$content = [System.IO.File]::ReadAllText($filePath, [System.Text.Encoding]::UTF8)
[System.IO.File]::WriteAllBytes($filePath, [System.Text.Encoding]::Unicode.GetBytes($content))
Write-Host "Converted: $filePath"
