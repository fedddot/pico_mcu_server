FROM python:3

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# COPY uart_proxy.py ./

ENV UART_TTY_PATH=/usr/src/app/ttyUART

CMD [ "/bin/bash" ]
# CMD [ "python", "./uart_proxy.py" ]