#!/bin/sh

for logs in `find /var/log -type f`; do > $logs; done
rm /var/log/*.log.?
rm /var/log/*.?
echo > /home/pi/.bash_history
echo Space before apt-get clena:
df -h
sudo apt-get clean
echo Space after apt-get clena
df -h

#find package sizes
dpkg-query -Wf '${Installed-Size}\t${Package}\n' | sort -n 


apt-get autoremove
apt-get clean 
echo Space after autoclean and clean
df -h


#show recommend packages with not being used as a dep
#aptitude search '?and( ?automatic(?reverse-recommends(?installed)), ?not(?automatic(?reverse-depends(?installed))) )' 

#deletes the above
#aptitude search '?and( ?automatic(?reverse-recommends(?installed)), ?not(?automatic(?reverse-depends(?installed))) )' | awk '{ print $3 }' | xargs dpkg -P


