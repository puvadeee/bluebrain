import pexpect
import time


cmd='/usr/local/bin/gatttool -b FC:E7:C2:AC:F0:FF -t random -I'
child = pexpect.spawn(cmd)
child.expect ('\[LE\]>')
child.sendline ("connect")

child.expect ('\[LE\]>')

child.sendline ('char-write-cmd 0x0011 "007F00"')
time.sleep(1)
child.sendline ('char-write-cmd 0x0011 "7F0000"')
time.sleep(1)
child.sendline ('char-write-cmd 0x0011 "FF7F00"')
time.sleep(1)
child.sendline ('char-write-cmd 0x0011 "7FFF00"')
time.sleep(1)

child.sendline ('char-write-cmd 0x0011 "7F7F00"')

