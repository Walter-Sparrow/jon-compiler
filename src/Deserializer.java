package src;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class Deserializer {
    public static List<Class<?>> searchClasses(String pkg) throws Exception {
        System.out.println("Searching classes...");
        final var annotatedClasses = new ArrayList<Class<?>>();

        final var classLoader = Thread.currentThread().getContextClassLoader();
        final var resource = classLoader.getResource(pkg);
        if (resource == null) {
            throw new IllegalArgumentException("Package not found: " + pkg);
        }

        final var directory = new File(resource.toURI());

        for (var file : directory.listFiles()) {
            if (file.getName().endsWith(".class")) {
                final var className = pkg + "." + file.getName().replace(".class", "");
                final var clazz = classLoader.loadClass(className);
                if (clazz.isAnnotationPresent(JsonSerializable.class)) {
                    annotatedClasses.add(clazz);
                }
            }
        }

        for (var c : annotatedClasses) {
            System.out.println("Found " + c.getName());
        }

        return annotatedClasses;
    }

    public static boolean isBoxedType(Object obj) {
        return obj instanceof Integer ||
                obj instanceof Long ||
                obj instanceof Double ||
                obj instanceof Float ||
                obj instanceof Character ||
                obj instanceof Short ||
                obj instanceof Byte ||
                obj instanceof Boolean;
    }

    public static boolean isValidType(Object obj) {
        return obj instanceof String || obj instanceof List ||
                isBoxedType(obj);
    }

    public static void printIndent(PrintWriter writer, int depth) {
        for (int i = 0; i < depth; i++) {
            writer.print("  ");
        }
    }

    public static void deserializeBasicType(PrintWriter writer, Object value) {
        if (value instanceof String) {
            writer.print("\"" + value + "\"");
        } else {
            writer.print(value);
        }
    }

    public static void deserializeBasicField(PrintWriter writer, String name, Object value, int depth) {
        printIndent(writer, depth);
        writer.print("\"" + name + "\": ");
        deserializeBasicType(writer, value);
    }

    public static void deserializeClass(PrintWriter writer, Class<?> clazz, int depth) throws Exception {
        writer.println("{");

        final var fields = clazz.getDeclaredFields();

        for (int i = 0; i < fields.length; i++) {
            final var field = fields[i];
            field.setAccessible(true);

            final var name = field.getName();
            final var value = field.get(null);

            if (field.isAnnotationPresent(JsonSerializable.class)) {
                if (i != 0) {
                    writer.print(",\n");
                }

                printIndent(writer, depth);
                writer.print("\"" + name + "\": ");
                final var fieldClazz = field.getType();
                deserializeClass(writer, fieldClazz, depth + 1);

                writer.println();
                printIndent(writer, depth);
                writer.print("}");
                continue;
            }

            if (!isValidType(value)) {
                continue;
            }

            if (i != 0) {
                writer.print(",\n");
            }

            if (isBoxedType(value) || value instanceof String) {
                deserializeBasicField(writer, name, value, depth);
                continue;
            }

            if (value instanceof List) {
                printIndent(writer, depth);
                writer.print("\"" + name + "\": [");
                final var list = (List<?>) value;
                for (int j = 0; j < list.size(); j++) {
                    final var item = list.get(j);
                    if (isBoxedType(item) || item instanceof String) {
                        deserializeBasicType(writer, item);
                    }

                    if (j != list.size() - 1) {
                        writer.print(", ");
                    }
                }
                writer.print("]");
                continue;
            }
        }
    }

    public static void deserialize(String pkg) throws Exception {
        final var classes = searchClasses(pkg);
        final var outDir = new File("out-configs");
        if (!outDir.exists()) {
            outDir.mkdir();
        }

        for (var c : classes) {
            System.out.println("Deserializing " + c.getName());

            final var file = new File(outDir, c.getSimpleName() + ".json");
            file.createNewFile();
            final var writer = new PrintWriter(file);

            deserializeClass(writer, c, 1);

            writer.println("\n}");
            writer.flush();
            writer.close();
        }
    }
}
