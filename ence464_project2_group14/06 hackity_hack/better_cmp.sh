#!/bin/bash
cmp <(sed 's/-0\.00000/0\.00000/g' $1) <(sed 's/-0\.00000/0\.00000/g' $2)