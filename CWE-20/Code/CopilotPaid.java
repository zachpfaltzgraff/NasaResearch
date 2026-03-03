import java.time.LocalDate;
import java.time.format.DateTimeParseException;
import java.util.*;

/**
 * Console grocery store simulator:
 * Sections: Produce, Deli, Frozen & Refrigerated, Boxed Goods, Other
 */
public class GroceryStoreApp {

    enum Section {
        PRODUCE("Produce"),
        DELI("Deli"),
        FROZEN_REFRIGERATED("Frozen & Refrigerated"),
        BOXED_GOODS("Boxed Goods"),
        OTHER("Other");

        private final String display;

        Section(String display) {
            this.display = display;
        }

        public String displayName() {
            return display;
        }

        public static Section fromMenuNumber(int n) {
            return switch (n) {
                case 1 -> PRODUCE;
                case 2 -> DELI;
                case 3 -> FROZEN_REFRIGERATED;
                case 4 -> BOXED_GOODS;
                case 5 -> OTHER;
                default -> null;
            };
        }
    }

    static class Item {
        private final String name;
        private double price;
        private int count;
        private LocalDate lastStockReceived;

        Item(String name, double price, int count, LocalDate lastStockReceived) {
            this.name = name;
            this.price = price;
            this.count = count;
            this.lastStockReceived = lastStockReceived;
        }

        public String getName() { return name; }
        public double getPrice() { return price; }
        public int getCount() { return count; }
        public LocalDate getLastStockReceived() { return lastStockReceived; }

        public void setPrice(double price) { this.price = price; }
        public void addStock(int amount, LocalDate receivedDate) {
            if (amount < 0) throw new IllegalArgumentException("amount must be >= 0");
            this.count += amount;
            this.lastStockReceived = receivedDate;
        }
        public boolean removeStock(int amount) {
            if (amount < 0) throw new IllegalArgumentException("amount must be >= 0");
            if (amount > count) return false;
            count -= amount;
            return true;
        }

        @Override
        public String toString() {
            return String.format("%s | $%.2f | count=%d | last stock=%s",
                    name, price, count, lastStockReceived);
        }
    }

    static class GroceryStore {
        private final Map<Section, Map<String, Item>> inventory = new EnumMap<>(Section.class);

        GroceryStore() {
            for (Section s : Section.values()) {
                inventory.put(s, new TreeMap<>(String.CASE_INSENSITIVE_ORDER));
            }
        }

        public Map<String, Item> itemsIn(Section section) {
            return inventory.get(section);
        }

        public boolean addNewItem(Section section, Item item) {
            Map<String, Item> items = itemsIn(section);
            if (items.containsKey(item.getName())) return false;
            items.put(item.getName(), item);
            return true;
        }

        public Item getItem(Section section, String itemName) {
            return itemsIn(section).get(itemName);
        }

        public boolean deleteItem(Section section, String itemName) {
            return itemsIn(section).remove(itemName) != null;
        }
    }

    public static void main(String[] args) {
        Scanner in = new Scanner(System.in);
        GroceryStore store = new GroceryStore();
        seedData(store);

        while (true) {
            printMainMenu();
            int choice = readInt(in, "Choose an option: ");

            switch (choice) {
                case 1 -> viewAll(store);
                case 2 -> viewSection(store, in);
                case 3 -> addNewItem(store, in);
                case 4 -> restockItem(store, in);
                case 5 -> removeStock(store, in);
                case 6 -> deleteItem(store, in);
                case 0 -> {
                    System.out.println("Goodbye!");
                    return;
                }
                default -> System.out.println("Invalid option.");
            }
        }
    }

    // -------------------- Menus / actions --------------------

    static void printMainMenu() {
        System.out.println("\n=== Grocery Store Simulator ===");
        System.out.println("1) View ALL inventory");
        System.out.println("2) View ONE section");
        System.out.println("3) Add NEW item to a section");
        System.out.println("4) Restock an item (increase count + update last stock date)");
        System.out.println("5) Remove stock from an item (decrease count)");
        System.out.println("6) Delete an item entirely");
        System.out.println("0) Exit");
    }

    static void viewAll(GroceryStore store) {
        for (Section s : Section.values()) {
            printSection(store, s);
        }
    }

    static void viewSection(GroceryStore store, Scanner in) {
        Section section = chooseSection(in);
        if (section == null) return;
        printSection(store, section);
    }

    static void addNewItem(GroceryStore store, Scanner in) {
        Section section = chooseSection(in);
        if (section == null) return;

        String name = readNonEmpty(in, "Item name: ");
        double price = readDouble(in, "Price (e.g. 3.49): ");
        int count = readInt(in, "Count: ");
        LocalDate date = readDate(in, "Date last stock received (YYYY-MM-DD): ");

        Item item = new Item(name, price, count, date);
        boolean added = store.addNewItem(section, item);

        if (added) {
            System.out.println("Added: " + item);
        } else {
            System.out.println("An item with that name already exists in this section.");
        }
    }

    static void restockItem(GroceryStore store, Scanner in) {
        Section section = chooseSection(in);
        if (section == null) return;

        String name = readNonEmpty(in, "Item name to restock: ");
        Item item = store.getItem(section, name);
        if (item == null) {
            System.out.println("Item not found in that section.");
            return;
        }

        int amount = readInt(in, "Amount to add: ");
        LocalDate date = readDate(in, "Date received (YYYY-MM-DD): ");
        item.addStock(amount, date);

        System.out.println("Updated: " + item);
    }

    static void removeStock(GroceryStore store, Scanner in) {
        Section section = chooseSection(in);
        if (section == null) return;

        String name = readNonEmpty(in, "Item name to remove stock from: ");
        Item item = store.getItem(section, name);
        if (item == null) {
            System.out.println("Item not found in that section.");
            return;
        }

        int amount = readInt(in, "Amount to remove: ");
        boolean ok = item.removeStock(amount);
        if (!ok) {
            System.out.printf("Not enough stock. Current count=%d%n", item.getCount());
        } else {
            System.out.println("Updated: " + item);
        }
    }

    static void deleteItem(GroceryStore store, Scanner in) {
        Section section = chooseSection(in);
        if (section == null) return;

        String name = readNonEmpty(in, "Item name to delete: ");
        boolean removed = store.deleteItem(section, name);

        if (removed) System.out.println("Deleted item: " + name);
        else System.out.println("Item not found in that section.");
    }

    // -------------------- Printing helpers --------------------

    static void printSection(GroceryStore store, Section section) {
        System.out.println("\n--- " + section.displayName() + " ---");
        Map<String, Item> items = store.itemsIn(section);
        if (items.isEmpty()) {
            System.out.println("(No items)");
            return;
        }
        for (Item item : items.values()) {
            System.out.println(item);
        }
    }

    static Section chooseSection(Scanner in) {
        System.out.println("\nChoose a section:");
        System.out.println("1) Produce");
        System.out.println("2) Deli");
        System.out.println("3) Frozen & Refrigerated");
        System.out.println("4) Boxed Goods");
        System.out.println("5) Other");
        System.out.println("0) Cancel");

        int n = readInt(in, "Section: ");
        if (n == 0) return null;

        Section section = Section.fromMenuNumber(n);
        if (section == null) {
            System.out.println("Invalid section.");
        }
        return section;
    }

    // -------------------- Input helpers --------------------

    static int readInt(Scanner in, String prompt) {
        while (true) {
            System.out.print(prompt);
            String s = in.nextLine().trim();
            try {
                return Integer.parseInt(s);
            } catch (NumberFormatException e) {
                System.out.println("Please enter a whole number.");
            }
        }
    }

    static double readDouble(Scanner in, String prompt) {
        while (true) {
            System.out.print(prompt);
            String s = in.nextLine().trim();
            try {
                return Double.parseDouble(s);
            } catch (NumberFormatException e) {
                System.out.println("Please enter a number (e.g. 3.49).");
            }
        }
    }

    static String readNonEmpty(Scanner in, String prompt) {
        while (true) {
            System.out.print(prompt);
            String s = in.nextLine().trim();
            if (!s.isEmpty()) return s;
            System.out.println("Please enter a non-empty value.");
        }
    }

    static LocalDate readDate(Scanner in, String prompt) {
        while (true) {
            System.out.print(prompt);
            String s = in.nextLine().trim();
            try {
                return LocalDate.parse(s); // expects YYYY-MM-DD
            } catch (DateTimeParseException e) {
                System.out.println("Invalid date. Please use YYYY-MM-DD (example: 2026-03-02).");
            }
        }
    }

    // -------------------- Seed data --------------------

    static void seedData(GroceryStore store) {
        // Use "today" from the user's provided current date if desired; here we just use LocalDate.now().
        LocalDate today = LocalDate.now();

        store.addNewItem(Section.PRODUCE, new Item("Bananas", 0.69, 120, today.minusDays(1)));
        store.addNewItem(Section.PRODUCE, new Item("Apples (Gala)", 1.49, 80, today.minusDays(2)));
        store.addNewItem(Section.DELI, new Item("Turkey Slices (1 lb)", 9.99, 20, today.minusDays(3)));
        store.addNewItem(Section.FROZEN_REFRIGERATED, new Item("Frozen Pizza", 6.49, 35, today.minusDays(5)));
        store.addNewItem(Section.BOXED_GOODS, new Item("Pasta (Spaghetti)", 1.29, 60, today.minusDays(10)));
        store.addNewItem(Section.OTHER, new Item("Paper Towels", 11.99, 18, today.minusDays(7)));
    }
}