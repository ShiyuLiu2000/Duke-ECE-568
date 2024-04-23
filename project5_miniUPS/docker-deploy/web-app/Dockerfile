FROM python:3
ENV PYTHONUNBUFFERED 1
# set time to UTC
# RUN ln -fs /usr/share/zoneinfo/UTC /etc/localtime
RUN mkdir /code
WORKDIR /code
ADD requirements.txt /code/
RUN pip install -r requirements.txt
ADD . /code/
