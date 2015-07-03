
Epiphany Design Verification Infrastructure
===========================================================

##Directory Structure

Directory       | Content
--------------- | ----------------------------
docs		| Installation guide and run direction
igen            | Instruction (program) generator
mgen            | Memory mapped transaction generation
comparegen      | Compares test output to expected results
epiphany_models | Building the device under test
scripts		| Various DV build and run scripts

##Pre-requisites
The infrastructure depends on SystemC and Verilator

##How to Install

System-C:
```
>> wget http://accellera.org/images/downloads/standards/systemc/systemc-2.3.1.tgz
>> tar -zxvf systemc-2.3.1.tgz
>> cd systemc-2.3.1
>> ../configure --prefix=/usr/local/systemc-2.3.1
>> make
>> make check
>> sudo make install
```
System-C
```
>> wget http://accellera.org/images/downloads/standards/systemc/scv-2.0.0.tgz
>> tar -zxvf scv-2.0.0.tgz
>> cd scv-2.0.0
>> ./configure --prefix=/usr/local/scv --with-systemc=/usr/local/systemc-2.3.1
>> make
>> make check
>> sudo mkdir /usr/local/scv
>> sudo make install

```


##How to Build

##How to Run
