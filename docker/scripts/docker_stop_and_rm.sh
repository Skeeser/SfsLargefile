#!/usr/bin/env bash

# 删除容器

echo "stop and rm docker" 
docker stop sfs_container > /dev/null
docker rm -v -f sfs_container > /dev/null