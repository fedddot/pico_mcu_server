#!/bin/bash

motor_x_id="motor_x"
motor_x_mapping="{ \
    \"a0\": 16, \
    \"a1\": 17, \
    \"b0\": 18, \
    \"b1\": 19, \
    \"en\": 15  \
}"
motor_y_id="motor_y"
motor_y_mapping="{ \
    \"a0\": 6, \
    \"a1\": 5, \
    \"b0\": 4, \
    \"b1\": 3, \
    \"en\": 7  \
}"
motor_z_id="motor_z"
motor_z_mapping="{ \
    \"a0\": 13, \
    \"a1\": 12, \
    \"b0\": 11, \
    \"b1\": 10, \
    \"en\": 14  \
}"

movement_id="linear_movement"
axes_assignment="{ \
    \"x\": \"motor_x\", \
    \"y\": \"motor_y\", \
    \"z\": \"motor_z\" \
}"
movement_cfg="{ \
    \"type\": 0, \
    \"steppers\": $axes_assignment, \
    \"time_multiplier\": 1000000 \
}"

movement_update_cfg1="{ \
    \"vector\": { \
        \"x\": -100, \
        \"y\": -200, \
        \"z\": -300 \
    }, \
    \"feed\": 1000 \
}"
movement_update_cfg2="{ \
    \"vector\": { \
        \"x\": 100, \
        \"y\": 200, \
        \"z\": 300 \
    }, \
    \"feed\": 2000 \
}"

echo
echo
echo "creating motor $motor_x_id"
curl -v -X POST http://localhost:5000/steppers -d "{ \"id\": \"$motor_x_id\", \"config\": $motor_x_mapping}"

echo
echo
echo "creating motor $motor_y_id"
curl -v -X POST http://localhost:5000/steppers -d "{ \"id\": \"$motor_y_id\", \"config\": $motor_y_mapping}"

echo
echo
echo "creating motor $motor_z_id"
curl -v -X POST http://localhost:5000/steppers -d "{ \"id\": \"$motor_z_id\", \"config\": $motor_z_mapping}"


echo
echo
echo "creating movement $movement_id"
curl -v -X POST http://localhost:5000/movements -d "{ \"id\": \"$movement_id\", \"config\": $movement_cfg}"

echo
echo
echo "updating movement $movement_id"
curl -v -X PUT "http://localhost:5000/movements/$movement_id" -d "$movement_update_cfg1"
curl -v -X PUT "http://localhost:5000/movements/$movement_id" -d "$movement_update_cfg2"

echo
echo
echo "deleting movement $movement_id"
curl -v -X DELETE "localhost:5000/movements/$movement_id"
echo
echo
echo "deleting motor $motor_x_id"
curl -v -X DELETE "localhost:5000/steppers/$motor_x_id"
echo
echo "deleting motor $motor_y_id"
curl -v -X DELETE "localhost:5000/steppers/$motor_y_id"
echo
echo "deleting motor $motor_z_id"
curl -v -X DELETE "localhost:5000/steppers/$motor_z_id"

echo
echo