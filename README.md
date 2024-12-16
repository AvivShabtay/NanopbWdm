# NanopbWdm

Needed to install this package:

```python
pip install protobuf grpcio-tools
```

Then run this command to generate the header and source files from my `.proto` file:

```bash
python ..\..\..\deps\nanopb\generator\nanopb_generator.py Configuration.proto
```

Download the protoc from the official Protobuf project, I used this one:
https://github.com/protocolbuffers/protobuf/releases/tag/v29.1

Then I used it to generate python files from my `.proto` file.

```bash
protoc --proto_path=. --python_out=. Configuration.proto
```

Generate serialized data using the python:

```python
In [1]: from Configuration_pb2 import SimpleMessage

In [5]: sm = SimpleMessage()

In [7]: sm.lucky_number = 10

In [9]: buffer = sm.SerializeToString()

In [11]: with open("config.bin", "wb") as f:
    ...:     f.write(buffer)

```

#### compiler requirements

I used this link to figure what are the requirements for using NanoPb in WDM driver: https://jpa.kapsi.fi/nanopb/docs/index.html#compiler-requirements.

I tried to comment the most of the content of: `extra/pb_syshdr.h` and see what is missing and to solve it specifically.

And I included the files: `pb_encode.c, pb_decode.c and pb_common.c` in the driver project so the compiler could compile them and then the linker can use them for the executable.

# Test the driver

Copy the `.sys` and `config.bin` to the VM. I copied it to `c:\temp`. Then install the driver and run it and you should see: `Your lucky number was 10!` in the debug view \ debugger console.
