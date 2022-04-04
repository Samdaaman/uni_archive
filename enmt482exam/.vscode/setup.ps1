$Folder = '.venv'

if (Test-Path -Path $Folder) {
    Write-Host "venv exists!" -ForegroundColor green
} else {
    Write-Host "venv doesn't exist. Making one for you now."  -ForegroundColor red
     python -m venv ".venv"
}
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt

"You are all good to go!"
"INFO: re-run this script to update packages."
"INFO: delete the .venv folder to re-initilize the script."