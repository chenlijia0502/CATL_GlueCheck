import win32com.client as com

def TotalSize(drive):
  """ Return the TotalSize of a shared drive [GB]"""
  try:
    fso = com.Dispatch("Scripting.FileSystemObject")
    drv = fso.GetDrive(drive)
    return drv.TotalSize/2**30
  except:
    return -1

def FreeSpace(drive):
  """ Return the FreeSpace of a shared drive [GB]"""
  try:
    fso = com.Dispatch("Scripting.FileSystemObject")
    drv = fso.GetDrive(drive)
    return drv.FreeSpace/2**30
  except:
    return -1
workstations = ['dolphins']
print ('Hard drive sizes:')

drive = "e:\\"
print ('*************************************************\n')
print ('TotalSize of %s = %f GB' % (drive, TotalSize(drive)))
print ('FreeSpace on %s = %f GB' % (drive, FreeSpace(drive)))
print ('*************************************************\n')

