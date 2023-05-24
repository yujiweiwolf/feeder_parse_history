FROM ubuntu:18.04
MAINTAINER bajizhh <bajizhh@gmail.com>

# --------------------------------------------------
# setup locale
RUN apt-get update && apt-get install -y --force-yes --no-install-recommends locales tzdata
RUN locale-gen en_US.UTF-8
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8
ENV LD_LIBRARY_PATH .:/usr/local/lib

# setup Timezone
RUN rm -rf /etc/localtime; ln -s /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
RUN echo Asia/Shanghai > /etc/timezone; dpkg-reconfigure -f noninteractive tzdata

# RUN echo "* - nofile 6553600" >> /etc/security/limits.conf
# clean
# RUN rm -rf /tmp/*; apt-get clean; rm -rf /var/lib/apt/lists/*

# ==================================================
WORKDIR /opt/guotaian_qts_feeder/bin
CMD ["./guotaian_qts_feeder"]

# --------------------------------------------------
RUN apt-get update && apt-get install -y libgssapi-krb5-2
RUN cd /opt/guotaian_qts_feeder; mkdir conf log data tmp
COPY guotaian_qts_feeder *.so*  GTAQTSApi*.ini /opt/guotaian_qts_feeder/bin/

RUN rm -rf /tmp/*; apt-get clean; rm -rf /var/lib/apt/lists/*
