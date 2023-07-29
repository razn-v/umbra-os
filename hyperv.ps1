$vmName = "umbraos"
$isoPath = "image.iso"

# Check if the VM is running and turn it off if that's the case
$vm = Get-VM -Name $vmName -ErrorAction SilentlyContinue
if ($vm.State -eq "Running") {
    Stop-VM -Name $vmName -TurnOff
}

# Insert the ISO file and start the VM 
Set-VMDvdDrive -VMName $vmName -Path $isoPath;
Start-VM -Name $vmName;

# Start the VMConnect session
$vmConnectProcess = Start-Process -FilePath "vmconnect.exe" -ArgumentList "localhost", $vmName -PassThru

# Wait for the VMConnect session to close
do {
    Start-Sleep -Milliseconds 500
} while (!$vmConnectProcess.HasExited)

# Stop the VM and eject the ISO file
Stop-VM -Name $vmName -TurnOff
Set-VMDvdDrive -VMName $vmName -Path "$null";
