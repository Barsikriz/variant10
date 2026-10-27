$ErrorActionPreference = 'Stop'
$root = Join-Path (Split-Path $PSScriptRoot -Parent) 'build/vfs'
$utf8 = New-Object System.Text.UTF8Encoding($false)
foreach ($path in @('minimal', 'several', 'deep/level1/level2/level3', 'deep/level1/empty', 'bad-motd')) {
    New-Item -ItemType Directory -Force (Join-Path $root $path) | Out-Null
}
[IO.File]::WriteAllText((Join-Path $root 'several/motd'), "Welcome to VFS!`n", $utf8)
[IO.File]::WriteAllText((Join-Path $root 'several/readme.txt'), "Sample file`n", $utf8)
[IO.File]::WriteAllBytes((Join-Path $root 'several/empty.txt'), [byte[]]@())
[IO.File]::WriteAllBytes((Join-Path $root 'several/binary.dat'), [byte[]]@(0, 1, 127, 128, 255))
[IO.File]::WriteAllText((Join-Path $root 'deep/motd'), "Deep VFS loaded.`n", $utf8)
[IO.File]::WriteAllText((Join-Path $root 'deep/level1/level2/level3/data.txt'), "Third level`n", $utf8)
[IO.File]::WriteAllBytes((Join-Path $root 'bad-motd/motd'), [byte[]]@(255, 254))
