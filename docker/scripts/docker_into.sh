#!/usr/bin/env bash

# 用来进入容器
docker exec \
    -u root \
    -it sfs_container \
    /bin/bash
