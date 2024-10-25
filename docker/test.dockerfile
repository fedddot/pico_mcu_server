FROM golang:1.23

ADD .. /usr/src/app/

WORKDIR /usr/src/app

RUN go mod download && go mod verify

CMD ["go", "test", "./..."]
