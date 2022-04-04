#!/bin/bash
#Author: Hank Wu.
if [ ! -d ".venv" ]
then
    echo "[STATUS] Configuring virtual envrionment....."
    python3 -m venv ".venv"  # "env" is the name of the environment here.
    source ".venv/bin/activate"
    echo "[STATUS] installing packages....."
fi


source ".venv/bin/activate"
#check package.
python3 -m pip install -r requirements.txt
firstThreeWord=$(echo "$result" | cut -d ":" -f 1)
if [[ "$firstThreeWord" == "Requirement already satisfied" ]] ; then
    echo "[STATUS] venv OK."
else
    echo $result
fi