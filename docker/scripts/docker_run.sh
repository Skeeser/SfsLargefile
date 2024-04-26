#!/usr/bin/env bash

# 构建容器

SFS_HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"

local_host="$(hostname)"
user="${USER}"
uid="$(id -u)"
group="$(id -g -n)"
gid="$(id -g)"


echo "stop and rm docker" 
docker stop sfs_container > /dev/null
docker rm -v -f sfs_container > /dev/null

echo "start docker"
docker run -it -d \
--name sfs_container \
-e DOCKER_USER="${user}" \
-e USER="${user}" \
-e DOCKER_USER_ID="${uid}" \
-e DOCKER_GRP="${group}" \
-e DOCKER_GRP_ID="${gid}" \
-v ${SFS_HOME_DIR}:/work \
--net host \
sfs_images