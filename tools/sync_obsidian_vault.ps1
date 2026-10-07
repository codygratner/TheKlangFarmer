<#
.SYNOPSIS
    Asymmetric Sync Bridge between The Klang Suite repository and a dedicated Obsidian Vault.

.DESCRIPTION
    Decouples Obsidian Sync (mobile/desktop) from Git.
    - Inbox (Vault -> Repo): Pulls mobile-captured notes from Vault/Inbox to docs/inbox/.
    - Docs (Repo -> Vault): Mirrors docs/ to Vault/Docs/ (excluding inbox/).
    - Telemetry: Generates real-time project dashboards and error logs in Vault/Telemetry/.
    - Heartbeat: Updates _sync_heartbeat.md in Vault root.

.PARAMETER VaultPath
    Target Obsidian Vault path. Defaults to "C:\Dev\TheKlangVault".

.PARAMETER Watch
    If specified, runs continuously in a loop watching for changes.

.PARAMETER IntervalSeconds
    Interval in seconds between watch iterations (default: 10).

.EXAMPLE
    .\tools\sync_obsidian_vault.ps1
    .\tools\sync_obsidian_vault.ps1 -Watch -IntervalSeconds 15
#>

[CmdletBinding()]
param(
    [string]$VaultPath = "C:\Dev\TheKlangVault",
    [switch]$Watch,
    [int]$IntervalSeconds = 10
)

$RepoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$RepoDocs = Join-Path $RepoRoot "docs"
$RepoInbox = Join-Path $RepoDocs "inbox"

$VaultInbox = Join-Path $VaultPath "Inbox"
$VaultDocs = Join-Path $VaultPath "Docs"
$VaultTelemetry = Join-Path $VaultPath "Telemetry"
$VaultCanvas = Join-Path $VaultPath "Canvas"

function Sync-Once {
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $timestamp = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss")
    $inboxSynced = 0
    $docsSynced = 0

    # Ensure all directories exist
    @($VaultPath, $VaultInbox, $VaultDocs, $VaultTelemetry, $VaultCanvas, $RepoDocs, $RepoInbox) | ForEach-Object {
        if (-not (Test-Path $_)) {
            New-Item -ItemType Directory -Path $_ -Force | Out-Null
        }
    }

    # 1. SYNC INBOX: Vault -> Repo (Never deletes from Vault)
    if (Test-Path $VaultInbox) {
        $inboxFiles = Get-ChildItem -Path $VaultInbox -File -Recurse | Where-Object { 
            -not ($_.Name.StartsWith("_sync_")) -and -not ($_.Name.StartsWith("."))
        }
        foreach ($file in $inboxFiles) {
            $relPath = $file.FullName.Substring($VaultInbox.Length).TrimStart('\', '/')
            $destPath = Join-Path $RepoInbox $relPath
            $destDir = Split-Path $destPath

            if (-not (Test-Path $destDir)) {
                New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            }

            $shouldCopy = $false
            if (-not (Test-Path $destPath)) {
                $shouldCopy = $true
            } else {
                $destItem = Get-Item $destPath
                if ($file.LastWriteTimeUtc -gt $destItem.LastWriteTimeUtc) {
                    $shouldCopy = $true
                }
            }

            if ($shouldCopy) {
                Copy-Item -Path $file.FullName -Destination $destPath -Force
                $inboxSynced++
                Write-Host "[INBOX -> REPO] $($file.Name)" -ForegroundColor Cyan
            }
        }
    }

    # 2. SYNC DOCS: Repo -> Vault (Excluding inbox and temp/hidden files)
    if (Test-Path $RepoDocs) {
        $docsFiles = Get-ChildItem -Path $RepoDocs -File -Recurse | Where-Object {
            $_.FullName -notmatch '[\\/]inbox[\\/]' -and
            $_.FullName -notmatch '[\\/]\.obsidian[\\/]' -and
            -not ($_.Name.StartsWith(".")) -and
            -not ($_.Name.StartsWith("_sync_")) -and
            $_.Extension -ne ".tmp"
        }

        foreach ($file in $docsFiles) {
            $relPath = $file.FullName.Substring($RepoDocs.Length).TrimStart('\', '/')
            $destPath = Join-Path $VaultDocs $relPath
            $destDir = Split-Path $destPath

            if (-not (Test-Path $destDir)) {
                New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            }

            $shouldCopy = $false
            if (-not (Test-Path $destPath)) {
                $shouldCopy = $true
            } else {
                $destItem = Get-Item $destPath
                if ($file.LastWriteTimeUtc -gt $destItem.LastWriteTimeUtc) {
                    $shouldCopy = $true
                }
            }

            if ($shouldCopy) {
                Copy-Item -Path $file.FullName -Destination $destPath -Force
                $docsSynced++
                Write-Host "[REPO -> DOCS] $relPath" -ForegroundColor DarkGray
            }
        }
    }

    # 3. TELEMETRY: Git Inspection
    $gitBranch = "unknown"
    $gitCommit = "unknown"
    $gitChanges = 0
    try {
        $gitBranch = (git -C $RepoRoot branch --show-current).Trim()
        $gitCommit = (git -C $RepoRoot log -1 --pretty=format:"%h - %s (%cr)").Trim()
        $statusLines = git -C $RepoRoot status --porcelain
        if ($statusLines) {
            $gitChanges = ($statusLines | Measure-Object).Count
        }
    } catch {
        Write-Warning "Failed to query Git status: $_"
    }

    # 4. TELEMETRY: DevLogger Inspection
    $devLogPath = "$env:LOCALAPPDATA\TheKlangSuite\dev.log"
    $recentErrors = @()
    $recentWarns = @()
    if (Test-Path $devLogPath) {
        try {
            $logLines = Get-Content -Path $devLogPath -Tail 200 -ErrorAction SilentlyContinue
            foreach ($line in $logLines) {
                if ($line -match "\[ERROR\]") { $recentErrors += $line }
                elseif ($line -match "\[WARN\]") { $recentWarns += $line }
            }
        } catch {}
    }

    # 5. WRITE TELEMETRY: Dashboard.md
    $statusEmoji = if ($recentErrors.Count -gt 0) { "[ALERT]" } else { "[OK]" }
    $q = [char]96 # Markdown backtick
    $dashboardLines = @(
        ("# {0} System Telemetry Dashboard" -f $statusEmoji),
        "> [!INFO] Live Status Feed",
        ("> **Last Synchronized:** {0}" -f $timestamp),
        ("> **Active Git Branch:** {0}{1}{0}" -f $q, $gitBranch),
        ("> **Latest Git Commit:** {0}" -f $gitCommit),
        ("> **Uncommitted Changes:** {0} files" -f $gitChanges),
        "",
        "---",
        "",
        "## Build & Deployment Status",
        ("- **VST3 System Deployment:** {0}C:\Program Files\Common Files\VST3{0}" -f $q),
        ("- **Local Artifacts:** {0}TheKlangSuite\current_build{0}" -f $q),
        ("- **Sync Stats:** {0} inbox notes ingested | {1} docs mirrored" -f $inboxSynced, $docsSynced),
        "",
        "---",
        "",
        "## Runtime Diagnostics (dev.log)"
    )

    if ($recentErrors.Count -gt 0) {
        $dashboardLines += ("> [!CAUTION] Active Errors Detected in dev.log ({0})" -f $recentErrors.Count)
        $dashboardLines += "> Review [[Active_Errors]] for complete tracebacks."
    } else {
        $dashboardLines += "> [!NOTE] Zero Active Errors"
        $dashboardLines += "> No [ERROR] log entries detected in the local developer log. Engine is running smoothly!"
    }

    if ($recentWarns.Count -gt 0) {
        $dashboardLines += ""
        $dashboardLines += ("### Recent Warnings ({0})" -f $recentWarns.Count)
        $dashboardLines += '```text'
        foreach ($w in ($recentWarns | Select-Object -Last 5)) { $dashboardLines += $w }
        $dashboardLines += '```'
    }

    Set-Content -Path (Join-Path $VaultTelemetry "Dashboard.md") -Value $dashboardLines -Encoding UTF8

    # 6. WRITE TELEMETRY: Active_Errors.md
    $errorsLines = @(
        "# Active Runtime Errors and Diagnostic Logs",
        ("**Last Checked:** {0}" -f $timestamp),
        ("**Source:** {0}{1}{0}" -f $q, $devLogPath),
        ""
    )

    if ($recentErrors.Count -gt 0) {
        $errorsLines += "> [!WARNING] The following errors were parsed from dev.log:"
        $errorsLines += ""
        $errorsLines += '```text'
        foreach ($e in ($recentErrors | Select-Object -Last 20)) { $errorsLines += $e }
        $errorsLines += '```'
    } else {
        $errorsLines += "> [!TIP] All Clean!"
        $errorsLines += "> No errors currently found in dev.log."
    }

    Set-Content -Path (Join-Path $VaultTelemetry "Active_Errors.md") -Value $errorsLines -Encoding UTF8

    # 7. WRITE HEARTBEAT: _sync_heartbeat.md
    $sw.Stop()
    $durationMs = [math]::Round($sw.Elapsed.TotalMilliseconds, 1)

    $heartbeatLines = @(
        "# Vault Sync Heartbeat",
        "- **Status:** ACTIVE & IN SYNC",
        ("- **Last Sync:** {0}" -f $timestamp),
        ("- **Duration:** {0} ms" -f $durationMs),
        ("- **Branch:** {0}{1}{0}" -f $q, $gitBranch),
        ("- **Inbox Ingested:** {0}" -f $inboxSynced),
        ("- **Docs Mirrored:** {0}" -f $docsSynced),
        ("- **Active Errors:** {0}" -f $recentErrors.Count)
    )

    Set-Content -Path (Join-Path $VaultPath "_sync_heartbeat.md") -Value $heartbeatLines -Encoding UTF8

    Write-Host "[OK] Sync pass complete (${durationMs}ms) - Inbox: $inboxSynced | Docs: $docsSynced | Errors: $($recentErrors.Count)" -ForegroundColor Green
}

# Execution Entry Point
if ($Watch) {
    Write-Host "[WATCH] Starting Obsidian Vault Sync Watcher (Interval: ${IntervalSeconds}s)... Press Ctrl+C to stop." -ForegroundColor Yellow
    while ($true) {
        try {
            Sync-Once
        } catch {
            Write-Warning "Sync iteration encountered error: $_"
        }
        Start-Sleep -Seconds $IntervalSeconds
    }
} else {
    Sync-Once
}
