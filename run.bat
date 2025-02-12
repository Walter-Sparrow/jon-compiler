@echo off

javac -d out example/*.java src/*.java
java -cp out example.Bootstrap
