import java.time.LocalDate;
import java.time.format.DateTimeParseException;
import java.util.*;

/**
 * Grocery Store Simulator
 * Sections:
 * 1) Produce
 * 2) Deli
 * 3) Frozen and Refrigerated
 * 4) Boxed Goods
 * 5) Other
 *
 * Features:
 * - View inventory (all sections or one section)
 * - Add a new item to a section
 * - Remove an item from a section
 * - Restock an existing item (adds quantity + updates lastStockDate)
 * - Sell/remove quantity from an existing item
 */
public class GroceryStoreApp {

    enum Section {
        PRODUCE("Produce"),
        DELI("Deli"),
        FROZEN_REFRIGERATED("Frozen and Refrigerated"),
        BOXED_GOODS("Boxed Goods"),
        OTHER("Other");

        private final String display;
        Section(String display) { this.display = display; }
        public String displayName() { return display; }

        public static Section fromMenuChoice(int choice) {
            return switch (choice) {
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
        private String name;
        private int priceCents; // store money as cents to avoid floating errors
        private int count;
        private LocalDate lastStockReceived;

        public Item(String name, int priceCents, int count, LocalDate lastStockReceived) {
            this.name = name;
            this.priceCents = priceCents;
            this.count = count;
            this.lastStockReceived = lastStockReceived;
        }

        public String getName() { return name; }
        public int getPriceCents() { return priceCents; }
        public int getCount() { return count; }
        public LocalDate getLastStockReceived() { return lastStockReceived; }

        public void setPriceCents(int priceCents) { this.priceCents = priceCents; }
        public void setLastStockReceived(LocalDate date) { this.lastStockReceived = date; }

        public void addCount(int qty, LocalDate dateReceived) {
            if (qty <= 0) return;
            this.count += qty;
            this.lastStockReceived = dateReceived;
        }

        public boolean removeCount(int qty) {
            if (qty <= 0) return false;
            if (qty > this.count) return false;
            this.count -= qty;
            return true;
        }

        public String priceAsDollars() {
            return String.format("$%d.%02d", priceCents / 100, Math.abs(priceCents % 100));
        }

        @Override
        public String toString() {
            return String.format(
                    "%-20s  Price: %-8s  Count: %-5d  Last Stock: %s",
                    name, priceAsDollars(), count, lastStockReceived
            );
        }
    }

    static class GroceryStore {
        // For each section, store items by normalized name for easy lookup
        private final Map<Section, Map<String, Item>> inventory = new EnumMap<>(Section.class);

        public GroceryStore() {
            for (Section s : Section.values()) {
                inventory.put(s, new LinkedHashMap<>());
            }
        }

        private static String keyOf(String name) {
            return name.trim().toLowerCase();
        }

        public void addNewItem(Section section, Item item) {
            inventory.get(section).put(keyOf(item.getName()), item);
        }

        public Item getItem(Section section, String name) {
            return inventory.get(section).get(keyOf(name));
        }

        public boolean removeItemCompletely(Section section, String name) {
            return inventory.get(section).remove(keyOf(name)) != null;
        }

        public Collection<Item> itemsInSection(Section section) {
            return inventory.get(section).values();
        }

        public void printSection(Section section) {
            System.out.println("\n=== " + section.displayName() + " ===");
            Map<String, Item> items = inventory.get(section);
            if (items.isEmpty()) {
                System.out.println("(No items in this section)");
                return;
            }
            int i = 1;
            for (Item item : items.values()) {
                System.out.printf("%2d) %s%n", i++, item);
            }
        }

        public void printAll() {
            for (Section s : Section.values()) {
                printSection(s);
            }
        }

        public void seedDemoData() {
            addNewItem(Section.PRODUCE, new Item("Apples", 199, 50, LocalDate.now().minusDays(2)));
            addNewItem(Section.PRODUCE, new Item("Bananas", 79, 80, LocalDate.now().minusDays(1)));
            addNewItem(Section.DELI, new Item("Turkey Slices", 599, 20, LocalDate.now().minusDays(3)));
            addNewItem(Section.FROZEN_REFRIGERATED, new Item("Frozen Pizza", 799, 25, LocalDate.now().minusDays(6)));
            addNewItem(Section.BOXED_GOODS, new Item("Cereal", 449, 35, LocalDate.now().minusDays(10)));
            addNewItem(Section.OTHER, new Item("Paper Towels", 999, 15, LocalDate.now().minusDays(4)));
        }
    }

    // ---------- Input helpers ----------
    private static int readInt(Scanner sc, String prompt, int min, int max) {
        while (true) {
            System.out.print(prompt);
            String line = sc.nextLine().trim();
            try {
                int val = Integer.parseInt(line);
                if (val < min || val > max) {
                    System.out.println("Please enter a number between " + min + " and " + max + ".");
                    continue;
                }
                return val;
            } catch (NumberFormatException e) {
                System.out.println("Invalid number. Try again.");
            }
        }
    }

    private static int readNonNegativeInt(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            String line = sc.nextLine().trim();
            try {
                int val = Integer.parseInt(line);
                if (val < 0) {
                    System.out.println("Please enter 0 or a positive number.");
                    continue;
                }
                return val;
            } catch (NumberFormatException e) {
                System.out.println("Invalid number. Try again.");
            }
        }
    }

    private static int readPriceCents(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            String line = sc.nextLine().trim();
            // Allow input like: 3.49 or 3 or 3.5
            try {
                if (line.contains(".")) {
                    String[] parts = line.split("\\.");
                    if (parts.length != 2) throw new NumberFormatException();
                    int dollars = Integer.parseInt(parts[0]);
                    String centsStr = parts[1];
                    if (centsStr.length() == 1) centsStr = centsStr + "0";
                    if (centsStr.length() != 2) throw new NumberFormatException();
                    int cents = Integer.parseInt(centsStr);
                    if (dollars < 0 || cents < 0 || cents > 99) throw new NumberFormatException();
                    return dollars * 100 + cents;
                } else {
                    int dollars = Integer.parseInt(line);
                    if (dollars < 0) throw new NumberFormatException();
                    return dollars * 100;
                }
            } catch (NumberFormatException e) {
                System.out.println("Invalid price. Examples: 3.49 or 3");
            }
        }
    }

    private static LocalDate readDate(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            String line = sc.nextLine().trim();
            try {
                return LocalDate.parse(line); // expects YYYY-MM-DD
            } catch (DateTimeParseException e) {
                System.out.println("Invalid date. Use YYYY-MM-DD (example: 2026-03-02).");
            }
        }
    }

    private static String readNonEmptyString(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            String line = sc.nextLine().trim();
            if (line.isEmpty()) {
                System.out.println("Please enter a non-empty value.");
                continue;
            }
            return line;
        }
    }

    private static Section chooseSection(Scanner sc) {
        System.out.println("\nChoose a section:");
        System.out.println("1) Produce");
        System.out.println("2) Deli");
        System.out.println("3) Frozen and Refrigerated");
        System.out.println("4) Boxed Goods");
        System.out.println("5) Other");
        int choice = readInt(sc, "Enter 1-5: ", 1, 5);
        return Section.fromMenuChoice(choice);
    }

    // ---------- Main menu actions ----------
    private static void addItemFlow(Scanner sc, GroceryStore store) {
        Section section = chooseSection(sc);

        String name = readNonEmptyString(sc, "Item name: ");
        int priceCents = readPriceCents(sc, "Item price (e.g., 3.49): ");
        int count = readNonNegativeInt(sc, "Item count: ");
        LocalDate date = readDate(sc, "Date last stock received (YYYY-MM-DD): ");

        Item existing = store.getItem(section, name);
        if (existing != null) {
            System.out.println("Item already exists in this section.");
            System.out.println("1) Update price");
            System.out.println("2) Restock (add quantity + update date)");
            System.out.println("3) Cancel");
            int c = readInt(sc, "Choose: ", 1, 3);
            if (c == 1) {
                existing.setPriceCents(priceCents);
                System.out.println("Price updated.");
            } else if (c == 2) {
                existing.addCount(count, date);
                existing.setPriceCents(priceCents); // common behavior: restock might update price too
                System.out.println("Restocked and updated.");
            } else {
                System.out.println("Cancelled.");
            }
            return;
        }

        store.addNewItem(section, new Item(name, priceCents, count, date));
        System.out.println("Item added.");
    }

    private static void removeItemFlow(Scanner sc, GroceryStore store) {
        Section section = chooseSection(sc);
        String name = readNonEmptyString(sc, "Item name to remove: ");

        Item item = store.getItem(section, name);
        if (item == null) {
            System.out.println("That item was not found in this section.");
            return;
        }

        System.out.println("Found: " + item);
        System.out.println("1) Remove some quantity (sell/remove units)");
        System.out.println("2) Remove item completely from section");
        System.out.println("3) Cancel");
        int c = readInt(sc, "Choose: ", 1, 3);

        if (c == 1) {
            int qty = readNonNegativeInt(sc, "Quantity to remove: ");
            if (qty == 0) {
                System.out.println("Nothing removed.");
                return;
            }
            if (!item.removeCount(qty)) {
                System.out.println("Not enough inventory to remove that many.");
                return;
            }
            System.out.println("Removed " + qty + ". Remaining: " + item.getCount());
            if (item.getCount() == 0) {
                System.out.println("Count is 0. You can remove it completely if you want.");
            }
        } else if (c == 2) {
            boolean removed = store.removeItemCompletely(section, name);
            System.out.println(removed ? "Item removed completely." : "Remove failed.");
        } else {
            System.out.println("Cancelled.");
        }
    }

    private static void restockFlow(Scanner sc, GroceryStore store) {
        Section section = chooseSection(sc);
        String name = readNonEmptyString(sc, "Item name to restock: ");

        Item item = store.getItem(section, name);
        if (item == null) {
            System.out.println("That item does not exist in this section. Use 'Add item' to create it.");
            return;
        }

        int qty = readNonNegativeInt(sc, "Quantity to add: ");
        LocalDate date = readDate(sc, "Date received (YYYY-MM-DD): ");
        int newPrice = readPriceCents(sc, "New price (enter same price if unchanged): ");

        item.addCount(qty, date);
        item.setPriceCents(newPrice);

        System.out.println("Restocked. Now: " + item);
    }

    private static void viewInventoryFlow(Scanner sc, GroceryStore store) {
        System.out.println("\n1) View ALL sections");
        System.out.println("2) View ONE section");
        int c = readInt(sc, "Choose: ", 1, 2);
        if (c == 1) store.printAll();
        else store.printSection(chooseSection(sc));
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        GroceryStore store = new GroceryStore();
        store.seedDemoData();

        while (true) {
            System.out.println("\n==============================");
            System.out.println("      Grocery Store Menu      ");
            System.out.println("==============================");
            System.out.println("1) View inventory");
            System.out.println("2) Add item to section");
            System.out.println("3) Remove item / remove quantity");
            System.out.println("4) Restock existing item");
            System.out.println("5) Exit");

            int choice = readInt(sc, "Choose 1-5: ", 1, 5);

            switch (choice) {
                case 1 -> viewInventoryFlow(sc, store);
                case 2 -> addItemFlow(sc, store);
                case 3 -> removeItemFlow(sc, store);
                case 4 -> restockFlow(sc, store);
                case 5 -> {
                    System.out.println("Goodbye!");
                    sc.close();
                    return;
                }
            }
        }
    }
}