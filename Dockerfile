# ---- Linux box ----
FROM ubuntu:bionic-20210512 AS poc_box
LABEL builder=true

ARG LINUX_TYPE_ID=debian
ENV LINUX_TYPE_ID=$LINUX_TYPE_ID

ENV RUN_APT_WITH='apt'

RUN apt update -y && \
	apt install -y apt-utils pkg-config software-properties-common git curl wget && \
	apt install -y gcc g++ cmake make gdb && \
	apt install -y libcapstone-dev && \
	git clone https://github.com/aclements/libelfin.git /libelfin && \
	cd /libelfin && \
	PREFIX=/usr make && \
	PREFIX=/usr make install
