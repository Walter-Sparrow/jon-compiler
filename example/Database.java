package example;

import java.util.List;

import src.JsonSerializable;

@JsonSerializable
public class Database {
    public static int port = 65537;
    public static boolean enabled = true;
    public static String name = "example";
    public static String version = "1.0";
    public static float weight = 69.1f;
    public static List<String> hosts = List.of("localhost", "127.0.0.1");
    public static List<Double> weights = List.of(1.0, 2.0, 11.0, 3.0);

    @JsonSerializable
    public static Connection connection;
}
