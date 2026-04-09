import java.io.*;
import java.lang.reflect.*;
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.util.*;

/**
 * SerInspector
 *
 * Usage:
 *   java SerInspector path/to/file.ser
 *
 * Notes:
 * - Deserializing untrusted .ser files can be dangerous. Run this locally, offline, and ideally in a sandbox.
 * - This tool prints:
 *   - Basic file info
 *   - Deserialization success/failure + root cause
 *   - Object class + serialVersionUID (if present)
 *   - A full reflection-based field dump (including private fields)
 *   - A "User-like summary" if it finds name/birthday/id/pin fields
 */
public class SerInspector {

    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Usage: java SerInspector <path-to-.ser-file>");
            System.exit(2);
        }

        File f = new File(args[0]);
        if (!f.exists() || !f.isFile()) {
            System.err.println("Error: file not found: " + f.getAbsolutePath());
            System.exit(2);
        }

        printFileInfo(f);

        Object obj;
        try {
            obj = readSerializedObject(f);
        } catch (Exception e) {
            System.err.println("\n=== DESERIALIZATION FAILED ===");
            printExceptionChain(e);
            System.exit(1);
            return;
        }

        System.out.println("\n=== DESERIALIZATION SUCCEEDED ===");
        if (obj == null) {
            System.out.println("Result: null object");
            return;
        }

        Class<?> cls = obj.getClass();
        System.out.println("Root object class: " + cls.getName());
        Long suid = tryGetSerialVersionUID(cls);
        if (suid != null) {
            System.out.println("serialVersionUID: " + suid);
        } else {
            System.out.println("serialVersionUID: (not declared or inaccessible)");
        }

        System.out.println("\n=== OBJECT TOSTRING() ===");
        try {
            System.out.println(String.valueOf(obj));
        } catch (Exception e) {
            System.out.println("(toString() threw exception)");
            printExceptionChain(e);
        }

        System.out.println("\n=== REFLECTION FIELD DUMP (recursive-ish) ===");
        dumpObject(obj, 0, new IdentityHashMap<>());

        System.out.println("\n=== USER-LIKE SUMMARY (best effort) ===");
        printUserLikeSummary(obj);

        System.out.println("\nDone.");
    }

    private static void printFileInfo(File f) {
        System.out.println("=== FILE INFO ===");
        System.out.println("Path: " + f.getAbsolutePath());
        System.out.println("Size: " + f.length() + " bytes");
        System.out.println("Last modified: " + Instant.ofEpochMilli(f.lastModified()));
    }

    /**
     * Reads exactly one object from the .ser file.
     */
    private static Object readSerializedObject(File f) throws IOException, ClassNotFoundException {
        // Buffered for performance and stability.
        try (InputStream in = new BufferedInputStream(new FileInputStream(f));
             ObjectInputStream ois = new ObjectInputStream(in)) {
            return ois.readObject();
        }
    }

    private static void printExceptionChain(Throwable t) {
        int i = 0;
        while (t != null) {
            System.err.println("[" + i + "] " + t.getClass().getName() + ": " + t.getMessage());
            // Print a few stack lines to keep output readable.
            StackTraceElement[] st = t.getStackTrace();
            for (int k = 0; k < Math.min(st.length, 12); k++) {
                System.err.println("      at " + st[k]);
            }
            if (st.length > 12) System.err.println("      ... (" + (st.length - 12) + " more)");
            t = t.getCause();
            i++;
            if (t != null) System.err.println("  Caused by:");
        }
    }

    private static Long tryGetSerialVersionUID(Class<?> cls) {
        try {
            Field f = cls.getDeclaredField("serialVersionUID");
            if (!Modifier.isStatic(f.getModifiers())) return null;
            f.setAccessible(true);
            Object v = f.get(null);
            if (v instanceof Long) return (Long) v;
        } catch (Exception ignored) {
        }
        return null;
    }

    private static void dumpObject(Object obj, int indent, IdentityHashMap<Object, Boolean> seen) {
        if (obj == null) {
            printlnIndent(indent, "null");
            return;
        }

        if (seen.containsKey(obj)) {
            printlnIndent(indent, "(already seen) " + safeClassName(obj));
            return;
        }

        Class<?> cls = obj.getClass();

        // Primitive-like / leaf values
        if (isLeaf(cls)) {
            printlnIndent(indent, safeClassName(obj) + " = " + String.valueOf(obj));
            return;
        }

        seen.put(obj, Boolean.TRUE);

        // Arrays
        if (cls.isArray()) {
            int len = Array.getLength(obj);
            printlnIndent(indent, "Array<" + cls.getComponentType().getName() + "> length=" + len);
            for (int i = 0; i < len; i++) {
                Object el = Array.get(obj, i);
                printlnIndent(indent + 1, "[" + i + "]:");
                dumpObject(el, indent + 2, seen);
            }
            return;
        }

        // Collections
        if (obj instanceof Collection<?> c) {
            printlnIndent(indent, cls.getName() + " (Collection) size=" + c.size());
            int i = 0;
            for (Object el : c) {
                printlnIndent(indent + 1, "[" + i + "]:");
                dumpObject(el, indent + 2, seen);
                i++;
            }
            return;
        }

        // Maps
        if (obj instanceof Map<?, ?> m) {
            printlnIndent(indent, cls.getName() + " (Map) size=" + m.size());
            int i = 0;
            for (Map.Entry<?, ?> e : m.entrySet()) {
                printlnIndent(indent + 1, "Entry[" + i + "].key:");
                dumpObject(e.getKey(), indent + 2, seen);
                printlnIndent(indent + 1, "Entry[" + i + "].value:");
                dumpObject(e.getValue(), indent + 2, seen);
                i++;
            }
            return;
        }

        // General object: print class then fields
        printlnIndent(indent, "Object: " + cls.getName());
        List<Field> fields = getAllFields(cls);

        if (fields.isEmpty()) {
            printlnIndent(indent + 1, "(no fields)");
            return;
        }

        for (Field f : fields) {
            if (Modifier.isStatic(f.getModifiers())) continue; // skip static fields
            f.setAccessible(true);
            Object v;
            try {
                v = f.get(obj);
            } catch (Exception e) {
                printlnIndent(indent + 1, f.getName() + " : (error reading) " + e.getClass().getSimpleName() + ": " + e.getMessage());
                continue;
            }

            String typeName = f.getType().getName();
            if (v == null || isLeaf(f.getType())) {
                printlnIndent(indent + 1, f.getName() + " : " + typeName + " = " + String.valueOf(v));
            } else {
                printlnIndent(indent + 1, f.getName() + " : " + typeName + " ->");
                dumpObject(v, indent + 2, seen);
            }
        }
    }

    private static List<Field> getAllFields(Class<?> cls) {
        List<Field> out = new ArrayList<>();
        for (Class<?> c = cls; c != null && c != Object.class; c = c.getSuperclass()) {
            Field[] fs = c.getDeclaredFields();
            out.addAll(Arrays.asList(fs));
        }
        out.sort(Comparator.comparing(Field::getName));
        return out;
    }

    private static boolean isLeaf(Class<?> cls) {
        return cls.isPrimitive()
                || Number.class.isAssignableFrom(cls)
                || CharSequence.class.isAssignableFrom(cls)
                || Boolean.class == cls
                || Character.class == cls
                || Enum.class.isAssignableFrom(cls)
                || Date.class.isAssignableFrom(cls)
                || cls.getName().startsWith("java.time.")
                || cls.getName().startsWith("java.net.")
                || cls.getName().startsWith("java.io.File")
                || cls == Class.class;
    }

    private static String safeClassName(Object obj) {
        try {
            return obj.getClass().getName();
        } catch (Exception e) {
            return "(unknown class)";
        }
    }

    private static void printlnIndent(int indent, String s) {
        System.out.println("  ".repeat(Math.max(0, indent)) + s);
    }

    /**
     * Tries to extract typical fields (by name) from the root object.
     * If the .ser contains a wrapper object, you can expand this to search nested objects too.
     */
    private static void printUserLikeSummary(Object obj) {
        Map<String, Object> found = new LinkedHashMap<>();

        // Common naming variations
        found.put("name", firstNonNull(
                readFieldByPossibleNames(obj, "name", "fullName", "username", "userName")
        ));

        found.put("birthday", firstNonNull(
                readFieldByPossibleNames(obj, "birthday", "birthDate", "dob", "dateOfBirth")
        ));

        found.put("idNumber", firstNonNull(
                readFieldByPossibleNames(obj, "idNumber", "id", "userId", "userid", "studentId", "studentID")
        ));

        found.put("pinCode", firstNonNull(
                readFieldByPossibleNames(obj, "pinCode", "pin", "pin_code", "pincode")
        ));

        Object name = found.get("name");
        Object birthday = found.get("birthday");
        Object idNumber = found.get("idNumber");
        Object pinCode = found.get("pinCode");

        System.out.println("name: " + formatValue(name));
        System.out.println("birthday: " + formatBirthday(birthday));
        System.out.println("idNumber: " + formatValue(idNumber));
        System.out.println("pinCode: " + formatPin(pinCode));

        // Validate pin if present
        String pinDigits = extractDigits(pinCode);
        if (pinDigits != null) {
            if (pinDigits.length() == 6) {
                System.out.println("pinCode validation: OK (6 digits)");
            } else {
                System.out.println("pinCode validation: WARNING (expected 6 digits, got " + pinDigits.length() + ")");
            }
        } else if (pinCode != null) {
            System.out.println("pinCode validation: WARNING (could not interpret as digits)");
        }
    }

    private static Object readFieldByPossibleNames(Object obj, String... names) {
        for (String n : names) {
            Object v = readField(obj, n);
            if (v != null) return v;
        }
        return null;
    }

    private static Object readField(Object obj, String fieldName) {
        if (obj == null) return null;

        for (Class<?> c = obj.getClass(); c != null && c != Object.class; c = c.getSuperclass()) {
            try {
                Field f = c.getDeclaredField(fieldName);
                if (Modifier.isStatic(f.getModifiers())) return null;
                f.setAccessible(true);
                return f.get(obj);
            } catch (NoSuchFieldException e) {
                // continue
            } catch (Exception e) {
                return "(error reading field " + fieldName + ": " + e.getClass().getSimpleName() + ": " + e.getMessage() + ")";
            }
        }

        // Also try a getter method if field doesn't exist
        String getter = "get" + Character.toUpperCase(fieldName.charAt(0)) + fieldName.substring(1);
        try {
            Method m = obj.getClass().getMethod(getter);
            if (Modifier.isStatic(m.getModifiers())) return null;
            return m.invoke(obj);
        } catch (Exception ignored) {
        }

        return null;
    }

    private static Object firstNonNull(Object v) {
        return v;
    }

    private static String formatValue(Object v) {
        if (v == null) return "(not found)";
        return String.valueOf(v);
    }

    private static String formatBirthday(Object v) {
        if (v == null) return "(not found)";
        if (v instanceof LocalDate ld) return ld.toString();
        if (v instanceof LocalDateTime ldt) return ldt.toLocalDate().toString();
        if (v instanceof Date d) return Instant.ofEpochMilli(d.getTime()).atZone(ZoneId.systemDefault()).toLocalDate().toString();
        if (v instanceof CharSequence cs) {
            // Try a couple common formats, otherwise return raw.
            String s = cs.toString().trim();
            for (String pattern : List.of("yyyy-MM-dd", "MM/dd/yyyy", "M/d/yyyy")) {
                try {
                    return LocalDate.parse(s, DateTimeFormatter.ofPattern(pattern)).toString();
                } catch (Exception ignored) {}
            }
            return s;
        }
        return String.valueOf(v);
    }

    private static String formatPin(Object v) {
        if (v == null) return "(not found)";
        String digits = extractDigits(v);
        if (digits == null) return String.valueOf(v);
        if (digits.length() == 6) return digits;
        // still show digits even if wrong length; it's a debug tool
        return digits + " (length=" + digits.length() + ")";
    }

    private static String extractDigits(Object v) {
        if (v == null) return null;
        if (v instanceof Number n) return String.valueOf(n.longValue());
        String s = String.valueOf(v);
        String digits = s.replaceAll("\\D+", "");
        return digits.isEmpty() ? null : digits;
    }
}