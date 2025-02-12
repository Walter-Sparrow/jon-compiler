# JON - Java Object Notation

JON (Java Object Notation) is a silly and experimental project that allows Java classes to act as configuration files, automatically serializing annotated static fields into JSON.

## Features

- Uses Java classes as configuration files.
- Serializes `static` fields into JSON format.
- Supports nested serialization with `@JsonSerializable` annotation.
- Handles primitive types, lists, and nested objects.
- **Not production-ready** – Just for fun and educational purposes!

## Example Usage

Define a Java class with annotated fields:

```java
@JsonSerializable
public class Database {
    public static String name = "example";
    public static String version = "1.0";
    public static int port = 3306;
    public static List<String> hosts = List.of("localhost", "127.0.0.1");
    public static List<Double> weights = List.of(1.0, 2.0, 3.0);

    @JsonSerializable
    public static Connection connection;
}
```

Generated JSON output:

```json
{
  "name": "example",
  "version": "1.0",
  "port": 3306,
  "hosts": ["localhost", "127.0.0.1"],
  "weights": [1.0, 2.0, 3.0],
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

## Installation & Usage

JON is not packaged as a library, but you can include the `@JsonSerializable` annotation and the serializer logic in your project.

## License

This project is just for fun and educational purposes. **Not suitable for production use!**
