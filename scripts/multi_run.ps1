$invocation = (Get-Variable MyInvocation).Value
$pwd = Split-Path $invocation.MyCommand.Path


$converter = ".\nmea2json.exe"

$FILES_INPUT = Get-ChildItem $pwd -Filter data\*.ubx
foreach($file_in in $FILES_INPUT)
{
    Write-Host "Parsing $file_in"
    $filename = $file_in.ToString().Split('.')
    $file_out = "data\"+$filename[0]+".json"
    &$converter -i data\$file_in -o $file_out
}