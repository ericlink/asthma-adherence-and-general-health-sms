#!/bin/bash

date

echo "Pushing site..."

rsync -e "ssh -l ubu-install-jeos -p 10222 -x -a -i /home/elink/.ssh/id_dsa.pub" \
--exclude '*nbproject' \
--exclude '*.log' \
--exclude '*swp' \
--exclude '*sh' \
--exclude '*~'  \
--exclude '.svn'  \
--exclude 'assets' \
-aruzitPL  \
./drupal/* \
ubu-install-jeos@67.18.182.74:/var/www/oat

date

echo "run fix-permissions on server!"

read
