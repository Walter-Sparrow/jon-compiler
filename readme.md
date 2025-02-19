# JON - Java Object Notation

JON (Java Object Notation) is a silly and experimental project that allows Java classes to act as configuration files, automatically serializing annotated static fields into JSON.

## Features

- Uses Java classes as configuration files.
- Serializes `static` fields into JSON format.
- Handles primitive types, arrays, and nested objects.
- **Not production-ready** – Just for fun and educational purposes!

## Example Usage

Define a Java class with static fields:

```java
public class Database {
    public static int port = 65537;
    public static boolean enabled = true;
    public static String name = "example";
    public static String version = "1.0";
    public static float weight = 69.1f;
    public static double[] sizes = { 1.0, 2.0, 3.0 };
    public static String[] hosts = { "localhost", "127.0.0.1" };
    public static Connection connection;
}
```

Generated JSON output:

```json
{
  "port": 65537,
  "enabled": 1,
  "name": "example",
  "version": "1.0",
  "weight": 69.1,
  "sizes": [1.0, 2.0, 3.0],
  "hosts": ["localhost", "127.0.0.1"],
  "connection": {
    "name": "mysql",
    "address": {
      "host": "localhost",
      "port": 6969
    }
  }
}
```

## Limitations

- Does **not** support non-static fields.
- Does **not** handle complex object graphs efficiently.
- **No validation or error handling** – it's just a fun experiment!

## License

This project is just for fun and educational purposes. **Not suitable for production use!**
