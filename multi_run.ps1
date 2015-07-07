$invocation = (Get-Variable MyInvocation).Value
$pwd = Split-Path $invocation.MyCommand.Path

$converter = ".\nmea_to_json.exe"

$FILES_INPUT = Get-ChildItem $pwd -Filter *.ubx
foreach($file_in in $FILES_INPUT)
{
    $filename = $file_in.ToString().Split('.')
    $file_out = $filename[0]+".json"
    &$converter -i $file_in -o $file_out
}