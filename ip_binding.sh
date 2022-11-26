#!/usr/bin/env bash

function main () {
    if (($# != 3)); then
        echo -e "\e[1;91mParameters: <kernelIP> <memoriaIP> <cpuIP>\e[0m"
        exit 1
    fi

    local -r kernelIP=$1
    local -r memoriaIP=$2
    local -r cpuIP=$3

    # Cambiar IP de memoria y cpu en kernel
    perl -pi -e "s/(?<=IP_MEMORIA=).*/${memoriaIP}/g" kernel/src/configs/*
    perl -pi -e "s/(?<=IP_CPU=).*/${cpuIP}/g" kernel/src/configs/*

    # Cambiar IP de memoria en cpu
    perl -pi -e "s/(?<=IP_MEMORIA=).*/${memoriaIP}/g" cpu/src/configs/*

    # Cambiar IP de kernel en consola
    perl -pi -e "s/(?<=IP_KERNEL=).*/${kernelIP}/g" consola/src/configs/*
}

main "$@"
