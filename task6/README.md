# Ex6
name : Tal Ornan id :209349356
name : Liel Vaknin id :207323049

## Notes
Before running each test prepend the statement `LD_LIBRARY_PATH=.` in order to find the right shared object to link.
In order to compile run `make all`.
This will build all the test and the shared object containing all the implementations (called `ex6.so`).

### Ex1
Test by running:
```
LD_LIBRARY_PATH=. ./queue_test
```

### Ex2
Test by running:
```
LD_LIBRARY_PATH=. ./active_object_test
```

### Ex3
* The `active_object_server` executable implements the requested logic by creating 3 `AO` objects.
* For each connection the data is received once and then the connection is closed.
* Running:
    ```
    LD_LIBRARY_PATH=. ./active_object_server
    ```

### Ex4
* In order the test run `LD_LIBRARY_PATH=. ./guard_test`.
* The test consists of two runs - one with `Guard` and a one with `Guard`.
This shows that the `Guard` is successful in preventing the race.

### Ex5
The test maps the file `README.md` to the memory and prints it using the class `MappedFile`.
After mapping the file it is printed to `STDOUT`.
The `MappedFile` instance is singleton.
```
LD_LIBRARY_PATH=. ./singleton_test
```

### Ex6
In order to run the server:
```
LD_LIBRARY_PATH=. ./selectserver
```
In order to run the client (you should run multiple clients):
```
LD_LIBRARY_PATH=. ./selectclient 127.0.0.1
```
