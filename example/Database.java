package example;

import java.util.List;

import src.JsonSerializable;

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
