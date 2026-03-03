import java.util.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;

// ─── GroceryItem ─────────────────────────────────────────────────────────────

class GroceryItem {
    private String name;
    private double price;
    private int count;
    private LocalDate lastStockDate;

    public GroceryItem(String name, double price, int count, LocalDate lastStockDate) {
        this.name = name;
        this.price = price;
        this.count = count;
        this.lastStockDate = lastStockDate;
    }

    public String getName()             { return name; }
    public double getPrice()            { return price; }
    public int getCount()               { return count; }
    public LocalDate getLastStockDate() { return lastStockDate; }

    public void setPrice(double price)           { this.price = price; }
    public void setCount(int count)              { this.count = count; }
    public void setLastStockDate(LocalDate date) { this.lastStockDate = date; }

    public String toDisplayString() {
        DateTimeFormatter fmt = DateTimeFormatter.ofPattern("MM/dd/yyyy");
        return String.format("  %-25s | Price: $%-8.2f | Count: %-6d | Last Stocked: %s",
                name, price, count, lastStockDate.format(fmt));
    }
}

// ─── StoreSection ─────────────────────────────────────────────────────────────

class StoreSection {
    private String name;
    private Map<String, GroceryItem> items;

    public StoreSection(String name) {
        this.name = name;
        this.items = new LinkedHashMap<>();
    }

    public String getName() { return name; }
    public Map<String, GroceryItem> getItems() { return items; }

    public void addItem(GroceryItem item) {
        items.put(item.getName().toLowerCase(), item);
        System.out.println("  [+] Added \"" + item.getName() + "\" to " + name + ".");
    }

    public boolean removeItem(String itemName) {
        GroceryItem removed = items.remove(itemName.toLowerCase());
        if (removed != null) {
            System.out.println("  [-] Removed \"" + removed.getName() + "\" from " + name + ".");
            return true;
        }
        System.out.println("  [!] Item \"" + itemName + "\" not found in " + name + ".");
        return false;
    }

    public GroceryItem findItem(String itemName) {
        return items.get(itemName.toLowerCase());
    }

    public void display() {
        System.out.println("\n+================================================================+");
        System.out.printf("|  Section: %-52s|%n", name);
        System.out.println("+================================================================+");
        if (items.isEmpty()) {
            System.out.println("  (No items in this section)");
        } else {
            System.out.printf("  %-25s | %-12s | %-8s | %s%n",
                    "Item Name", "Price", "Count", "Last Stocked");
            System.out.println("  " + "-".repeat(70));
            for (GroceryItem item : items.values()) {
                System.out.println(item.toDisplayString());
            }
        }
    }
}

// ─── GroceryStore (Main) ──────────────────────────────────────────────────────

public class GroceryStore {

    private static final Map<String, StoreSection> sections = new LinkedHashMap<>();
    private static final Scanner scanner = new Scanner(System.in);
    private static final DateTimeFormatter DATE_FMT = DateTimeFormatter.ofPattern("MM/dd/yyyy");

    public static void main(String[] args) {
        initializeSections();
        seedDefaultItems();
        runMainMenu();
    }

    // ── Initialization ────────────────────────────────────────────────────────

    private static void initializeSections() {
        String[] names = {"Produce", "Deli", "Frozen and Refrigerated", "Boxed Goods", "Other"};
        for (String name : names) {
            sections.put(name.toLowerCase(), new StoreSection(name));
        }
    }

    private static void seedDefaultItems() {
        LocalDate today     = LocalDate.now();
        LocalDate yesterday = today.minusDays(1);
        LocalDate lastWeek  = today.minusDays(7);

        // Produce
        addDefault("Produce", "Apples",   1.49, 120, lastWeek);
        addDefault("Produce", "Bananas",  0.59, 200, yesterday);
        addDefault("Produce", "Broccoli", 1.99,  85, today);
        addDefault("Produce", "Carrots",  0.99, 150, lastWeek);
        addDefault("Produce", "Spinach",  2.49,  60, yesterday);

        // Deli
        addDefault("Deli", "Turkey Breast",  7.99, 40, today);
        addDefault("Deli", "Ham",            6.49, 35, yesterday);
        addDefault("Deli", "Swiss Cheese",   5.99, 50, today);
        addDefault("Deli", "Roast Beef",     9.99, 25, lastWeek);
        addDefault("Deli", "Chicken Salad",  4.99, 30, today);

        // Frozen and Refrigerated
        addDefault("Frozen and Refrigerated", "Frozen Pizza",    5.49, 80, lastWeek);
        addDefault("Frozen and Refrigerated", "Ice Cream",       3.99, 60, lastWeek);
        addDefault("Frozen and Refrigerated", "Orange Juice",    3.49, 90, yesterday);
        addDefault("Frozen and Refrigerated", "Butter",          4.29, 70, yesterday);
        addDefault("Frozen and Refrigerated", "Frozen Burritos", 2.99, 55, today);

        // Boxed Goods
        addDefault("Boxed Goods", "Cereal",       3.99, 100, lastWeek);
        addDefault("Boxed Goods", "Pasta",        1.49,  80, lastWeek);
        addDefault("Boxed Goods", "Mac & Cheese", 1.29, 200, lastWeek);
        addDefault("Boxed Goods", "Rice",         2.49, 120, yesterday);
        addDefault("Boxed Goods", "Crackers",     2.99,  90, yesterday);

        // Other
        addDefault("Other", "Dish Soap",     2.79, 60, lastWeek);
        addDefault("Other", "Paper Towels",  4.99, 40, lastWeek);
        addDefault("Other", "Aluminum Foil", 3.49, 55, yesterday);
        addDefault("Other", "Trash Bags",    5.99, 35, yesterday);
        addDefault("Other", "Sponges",       1.99, 80, today);
    }

    private static void addDefault(String section, String name,
                                   double price, int count, LocalDate date) {
        sections.get(section.toLowerCase()).getItems()
                .put(name.toLowerCase(), new GroceryItem(name, price, count, date));
    }

    // ── Main Menu ─────────────────────────────────────────────────────────────

    private static void runMainMenu() {
        while (true) {
            printBanner();
            System.out.println("  1. View all sections");
            System.out.println("  2. View a specific section");
            System.out.println("  3. Add an item to a section");
            System.out.println("  4. Remove an item from a section");
            System.out.println("  5. Update an item's details");
            System.out.println("  6. Search for an item");
            System.out.println("  0. Exit");
            System.out.print("\n  Choose an option: ");

            switch (scanner.nextLine().trim()) {
                case "1" -> viewAllSections();
                case "2" -> viewSection();
                case "3" -> addItem();
                case "4" -> removeItem();
                case "5" -> updateItem();
                case "6" -> searchItem();
                case "0" -> { System.out.println("\n  Goodbye!\n"); return; }
                default  -> System.out.println("  Invalid option. Please try again.");
            }
        }
    }

    private static void printBanner() {
        System.out.println("\n+================================================================+");
        System.out.println("|              JAVA GROCERY STORE MANAGER                       |");
        System.out.println("+================================================================+");
    }

    // ── Actions ───────────────────────────────────────────────────────────────

    private static void viewAllSections() {
        for (StoreSection section : sections.values()) {
            section.display();
        }
        pause();
    }

    private static void viewSection() {
        StoreSection section = pickSection();
        if (section != null) { section.display(); pause(); }
    }

    private static void addItem() {
        StoreSection section = pickSection();
        if (section == null) return;

        System.out.print("  Item name: ");
        String name = scanner.nextLine().trim();
        if (name.isEmpty()) { System.out.println("  Name cannot be empty."); return; }

        double    price = promptDouble("  Price ($): ");
        int       count = promptInt("  Count (quantity): ");
        LocalDate date  = promptDate("  Last stock date (MM/DD/YYYY, or Enter for today): ");

        section.addItem(new GroceryItem(name, price, count, date));
    }

    private static void removeItem() {
        StoreSection section = pickSection();
        if (section == null) return;

        System.out.print("  Item name to remove: ");
        section.removeItem(scanner.nextLine().trim());
    }

    private static void updateItem() {
        StoreSection section = pickSection();
        if (section == null) return;

        System.out.print("  Item name to update: ");
        String name = scanner.nextLine().trim();
        GroceryItem item = section.findItem(name);

        if (item == null) {
            System.out.println("  [!] \"" + name + "\" not found in " + section.getName() + ".");
            return;
        }

        System.out.println("\n  Current: " + item.toDisplayString());
        System.out.println("\n  What to update?  1) Price  2) Count  3) Date  4) All  0) Cancel");
        System.out.print("  Choice: ");

        switch (scanner.nextLine().trim()) {
            case "1" -> item.setPrice(promptDouble("  New price ($): "));
            case "2" -> item.setCount(promptInt("  New count: "));
            case "3" -> item.setLastStockDate(promptDate("  New date (MM/DD/YYYY): "));
            case "4" -> {
                item.setPrice(promptDouble("  New price ($): "));
                item.setCount(promptInt("  New count: "));
                item.setLastStockDate(promptDate("  New date (MM/DD/YYYY): "));
            }
            case "0" -> { return; }
            default  -> { System.out.println("  Invalid option."); return; }
        }
        System.out.println("  [+] \"" + item.getName() + "\" updated successfully.");
    }

    private static void searchItem() {
        System.out.print("  Search item name: ");
        String query = scanner.nextLine().trim().toLowerCase();
        boolean found = false;

        System.out.println();
        // Exact match
        for (StoreSection section : sections.values()) {
            GroceryItem item = section.findItem(query);
            if (item != null) {
                System.out.println("  Found in [" + section.getName() + "]:");
                System.out.println(item.toDisplayString());
                found = true;
            }
        }
        // Partial matches
        if (!found) {
            System.out.println("  No exact match found. Partial matches:");
            for (StoreSection section : sections.values()) {
                for (Map.Entry<String, GroceryItem> e : section.getItems().entrySet()) {
                    if (e.getKey().contains(query)) {
                        System.out.println("  [" + section.getName() + "]: "
                                + e.getValue().toDisplayString());
                        found = true;
                    }
                }
            }
        }
        if (!found) System.out.println("  No items found matching \"" + query + "\".");
        pause();
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    private static StoreSection pickSection() {
        System.out.println("\n  Select a section:");
        List<StoreSection> list = new ArrayList<>(sections.values());
        for (int i = 0; i < list.size(); i++)
            System.out.printf("  %d. %s%n", i + 1, list.get(i).getName());
        System.out.print("  Choice (1-" + list.size() + "): ");
        try {
            int idx = Integer.parseInt(scanner.nextLine().trim()) - 1;
            if (idx >= 0 && idx < list.size()) return list.get(idx);
        } catch (NumberFormatException ignored) {}
        System.out.println("  Invalid selection.");
        return null;
    }

    private static double promptDouble(String prompt) {
        while (true) {
            System.out.print(prompt);
            try { return Double.parseDouble(scanner.nextLine().trim()); }
            catch (NumberFormatException e) { System.out.println("  Please enter a valid number."); }
        }
    }

    private static int promptInt(String prompt) {
        while (true) {
            System.out.print(prompt);
            try { return Integer.parseInt(scanner.nextLine().trim()); }
            catch (NumberFormatException e) { System.out.println("  Please enter a valid integer."); }
        }
    }

    private static LocalDate promptDate(String prompt) {
        while (true) {
            System.out.print(prompt);
            String input = scanner.nextLine().trim();
            if (input.isEmpty()) return LocalDate.now();
            try { return LocalDate.parse(input, DATE_FMT); }
            catch (DateTimeParseException e) { System.out.println("  Use MM/DD/YYYY format."); }
        }
    }

    private static void pause() {
        System.out.print("\n  Press Enter to continue...");
        scanner.nextLine();
    }
}