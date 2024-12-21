#!/bin/bash

# MySQL connection settings
MYSQL_HOST="localhost"
MYSQL_USER="root"
MYSQL_PASS="root"
MYSQL_DB="trade_history"

# Create balance_history tables
for i in `seq 0 99`
do
    echo "Creating table balance_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB -e "CREATE TABLE IF NOT EXISTS balance_history_$i LIKE balance_history;"
done

# Create order_history tables
for i in `seq 0 99`
do
    echo "Creating table order_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB -e "CREATE TABLE IF NOT EXISTS order_history_$i LIKE order_history;"
done

# Create order_detail tables
for i in `seq 0 99`
do
    echo "Creating table order_detail_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB -e "CREATE TABLE IF NOT EXISTS order_detail_$i LIKE order_detail;"
done

# Create deal_history tables
for i in `seq 0 99`
do
    echo "Creating table deal_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB -e "CREATE TABLE IF NOT EXISTS deal_history_$i LIKE deal_history;"
done

# Create user_deal_history tables
for i in `seq 0 99`
do
    echo "Creating table user_deal_history_$i"
    mysql -h$MYSQL_HOST -u$MYSQL_USER -p$MYSQL_PASS $MYSQL_DB -e "CREATE TABLE IF NOT EXISTS user_deal_history_$i LIKE user_deal_history;"
done
