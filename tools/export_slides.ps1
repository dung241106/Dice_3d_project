param([string]$InputPath = (Join-Path $PSScriptRoot '..\docs\SLIDE_THUYET_TRINH.pptx'))
$ErrorActionPreference = 'Stop'
$resolved = (Resolve-Path -LiteralPath $InputPath).Path
$pdfPath = [System.IO.Path]::ChangeExtension($resolved, '.pdf')
$previewPath = Join-Path (Split-Path -Parent $resolved) 'slide_previews'
New-Item -ItemType Directory -Force -Path $previewPath | Out-Null
$renderDirectory = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\tmp'))
New-Item -ItemType Directory -Force -Path $renderDirectory | Out-Null
$renderInput = Join-Path $renderDirectory 'render_slides_input.pptx'
Copy-Item -LiteralPath $resolved -Destination $renderInput -Force
$powerpoint = $null
$presentation = $null
$hadPresentations = $false
try {
    $powerpoint = New-Object -ComObject PowerPoint.Application
    $hadPresentations = $powerpoint.Presentations.Count -gt 0
    $presentation = $powerpoint.Presentations.Open($renderInput, $true, $false, $false)
    $presentation.SaveAs($pdfPath, 32)
    for ($index = 1; $index -le $presentation.Slides.Count; $index++) {
        $imagePath = Join-Path $previewPath ('slide_{0:00}.png' -f $index)
        $presentation.Slides.Item($index).Export($imagePath, 'PNG', 1600, 900)
    }
    Write-Output ('Rendered {0} PowerPoint slides and PDF' -f $presentation.Slides.Count)
} finally {
    if ($presentation) { $presentation.Close(); [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($presentation) }
    if ($powerpoint) {
        if (-not $hadPresentations) { $powerpoint.Quit() }
        [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($powerpoint)
    }
}
