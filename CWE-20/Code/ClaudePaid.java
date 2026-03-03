import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.*;

// =============================================================================
//  GroceryItem — a single product on the shelf
// =============================================================================
class GroceryItem {
    private String    name;
    private double    price;
    private int       count;
    private LocalDate lastStockDate;

    public GroceryItem(String name, double price, int count, LocalDate lastStockDate) {
        this.name = name; this.price = price;
        this.count = count; this.lastStockDate = lastStockDate;
    }

    public String    getName()          { return name; }
    public double    getPrice()         { return price; }
    public int       getCount()         { return count; }
    public LocalDate getLastStockDate() { return lastStockDate; }

    public void setPrice(double price)           { this.price = price; }
    public void setCount(int count)              { this.count = count; }
    public void setLastStockDate(LocalDate date) { this.lastStockDate = date; }

    @Override
    public String toString() {
        DateTimeFormatter fmt = DateTimeFormatter.ofPattern("MM/dd/yyyy");
        return String.format("  %-30s  $%-8.2f  Count: %-6d  Last Stocked: %s",
                name, price, count, lastStockDate.format(fmt));
    }
}

// =============================================================================
//  Section — one of the five store areas
// =============================================================================
class Section {
    private final String name;
    private final Map<String, GroceryItem> items = new LinkedHashMap<>();

    public Section(String name) { this.name = name; }

    public String getName()                   { return name; }
    public Collection<GroceryItem> getItems() { return items.values(); }

    public boolean addItem(GroceryItem item) {
        String key = item.getName().toLowerCase();
        if (items.containsKey(key)) return false;
        items.put(key, item);
        return true;
    }

    public boolean removeItem(String itemName) {
        return items.remove(itemName.toLowerCase()) != null;
    }

    public Optional<GroceryItem> findItem(String itemName) {
        return Optional.ofNullable(items.get(itemName.toLowerCase()));
    }

    public void display() {
        String header = " " + name + " ";
        String bar = "─".repeat(Math.max(1, 63 - header.length() - 1));
        System.out.println("\n┌─" + header + bar + "┐");
        if (items.isEmpty()) System.out.println("  (no items in this section)");
        else items.values().forEach(System.out::println);
        System.out.println("└" + "─".repeat(63) + "┘");
    }
}

// =============================================================================
//  Store — owns the five sections and seeds sample data
// =============================================================================
class Store {
    private final String storeName;
    private final List<String>         sectionOrder = new ArrayList<>();
    private final Map<String, Section> sections     = new LinkedHashMap<>();

    static final List<String> DEFAULT_SECTIONS =
            List.of("Produce", "Deli", "Frozen and Refrigerated", "Boxed Goods", "Other");

    public Store(String storeName) {
        this.storeName = storeName;
        for (String s : DEFAULT_SECTIONS) {
            sectionOrder.add(s);
            sections.put(s.toLowerCase(), new Section(s));
        }
        seedData();
    }

    public String getStoreName()          { return storeName; }
    public List<String> getSectionNames() { return Collections.unmodifiableList(sectionOrder); }

    public Optional<Section> getSection(String name) {
        return Optional.ofNullable(sections.get(name.toLowerCase()));
    }

    public Optional<Object[]> findAnywhere(String itemName) {
        for (Section sec : sections.values()) {
            Optional<GroceryItem> item = sec.findItem(itemName);
            if (item.isPresent()) return Optional.of(new Object[]{sec, item.get()});
        }
        return Optional.empty();
    }

    public void displayAll() {
        System.out.println("\n══════════════════════════════════════════════════════════════════");
        System.out.printf("  %s — Full Inventory%n", storeName);
        System.out.println("══════════════════════════════════════════════════════════════════");
        sections.values().forEach(Section::display);
    }

    private void seedData() {
        LocalDate today = LocalDate.now(), yesterday = today.minusDays(1),
                  lastWeek = today.minusDays(7), twoWeeks = today.minusDays(14);

        Section produce = sections.get("produce");
        produce.addItem(new GroceryItem("Apples (bag)",         1.99, 40, lastWeek));
        produce.addItem(new GroceryItem("Bananas (bunch)",       0.59, 60, yesterday));
        produce.addItem(new GroceryItem("Baby Carrots (bag)",    1.29, 30, today));
        produce.addItem(new GroceryItem("Romaine Lettuce",       2.49, 25, today));
        produce.addItem(new GroceryItem("Tomatoes (lb)",         1.79, 35, yesterday));

        Section deli = sections.get("deli");
        deli.addItem(new GroceryItem("Turkey Breast (lb)",       8.99, 20, today));
        deli.addItem(new GroceryItem("Smoked Ham (lb)",          7.49, 15, today));
        deli.addItem(new GroceryItem("American Cheese (lb)",     5.99, 22, yesterday));
        deli.addItem(new GroceryItem("Roast Beef (lb)",         10.99, 10, today));
        deli.addItem(new GroceryItem("Coleslaw (lb)",            3.49, 18, yesterday));

        Section frozen = sections.get("frozen and refrigerated");
        frozen.addItem(new GroceryItem("Whole Milk (gallon)",    4.29, 30, yesterday));
        frozen.addItem(new GroceryItem("Large Eggs (dozen)",     3.79, 50, today));
        frozen.addItem(new GroceryItem("Butter (lb)",            5.49, 25, lastWeek));
        frozen.addItem(new GroceryItem("Frozen Pizza",           6.99, 20, twoWeeks));
        frozen.addItem(new GroceryItem("Ice Cream (pint)",       4.99, 15, lastWeek));

        Section boxed = sections.get("boxed goods");
        boxed.addItem(new GroceryItem("Spaghetti (16 oz)",       1.49, 45, twoWeeks));
        boxed.addItem(new GroceryItem("Cereal (family size)",    4.99, 30, lastWeek));
        boxed.addItem(new GroceryItem("Rice (5 lb bag)",         3.99, 35, twoWeeks));
        boxed.addItem(new GroceryItem("Mac and Cheese",          1.19, 60, lastWeek));
        boxed.addItem(new GroceryItem("Chicken Broth (32 oz)",   2.49, 40, twoWeeks));

        Section other = sections.get("other");
        other.addItem(new GroceryItem("Paper Towels (6-pack)",   8.99, 20, twoWeeks));
        other.addItem(new GroceryItem("Dish Soap",               3.49, 25, lastWeek));
        other.addItem(new GroceryItem("Laundry Detergent",      12.99, 15, twoWeeks));
        other.addItem(new GroceryItem("Trash Bags (20-ct)",      5.99, 18, twoWeeks));
        other.addItem(new GroceryItem("Aluminum Foil",           4.29, 22, lastWeek));
    }
}

// =============================================================================
//  GroceryStore — main class / CLI
// =============================================================================
public class GroceryStore {

    private static final Scanner sc = new Scanner(System.in);
    private static Store store;

    public static void main(String[] args) {
        store = new Store("FreshMart Grocery");
        printBanner();
        boolean running = true;
        while (running) {
            printMainMenu();
            int choice = readInt("  Choice: ", 1, 7);
            System.out.println();
            switch (choice) {
                case 1 -> store.displayAll();
                case 2 -> viewSection();
                case 3 -> addItem();
                case 4 -> removeItem();
                case 5 -> updateItem();
                case 6 -> searchItem();
                case 7 -> { System.out.println("  Goodbye! Have a fresh day!\n"); running = false; }
            }
        }
    }

    private static void printBanner() {
        System.out.println("╔══════════════════════════════════════════════════════════════════╗");
        System.out.println("║          FreshMart Grocery Store Simulator                       ║");
        System.out.println("╚══════════════════════════════════════════════════════════════════╝");
    }

    private static void printMainMenu() {
        System.out.println("\n┌────────────────── MAIN MENU ───────────────────┐");
        System.out.println("│  1. View entire store inventory                 │");
        System.out.println("│  2. View a specific section                     │");
        System.out.println("│  3. Add an item to a section                    │");
        System.out.println("│  4. Remove an item from a section               │");
        System.out.println("│  5. Update an item's details                    │");
        System.out.println("│  6. Search for an item across all sections      │");
        System.out.println("│  7. Exit                                        │");
        System.out.println("└─────────────────────────────────────────────────┘");
    }

    private static void viewSection() {
        Section sec = pickSection("View which section?");
        if (sec != null) sec.display();
    }

    private static void addItem() {
        Section sec = pickSection("Add item to which section?");
        if (sec == null) return;

        System.out.print("  Item name         : ");
        String name = sc.nextLine().trim();
        if (name.isEmpty()) { System.out.println("  ERROR: Name cannot be empty."); return; }
        if (sec.findItem(name).isPresent()) {
            System.out.println("  ERROR: '" + name + "' already exists in " + sec.getName() + ".");
            return;
        }

        double    price = readDouble("  Price ($)         : ");
        int       count = readInt   ("  Count             : ", 0, Integer.MAX_VALUE);
        LocalDate date  = readDate  ("  Last stocked (MM/DD/YYYY, blank = today): ");

        sec.addItem(new GroceryItem(name, price, count, date));
        System.out.println("  SUCCESS: '" + name + "' added to " + sec.getName() + ".");
    }

    private static void removeItem() {
        Section sec = pickSection("Remove item from which section?");
        if (sec == null) return;
        sec.display();

        System.out.print("\n  Item name to remove: ");
        String name = sc.nextLine().trim();

        if (sec.removeItem(name))
            System.out.println("  SUCCESS: '" + name + "' removed from " + sec.getName() + ".");
        else
            System.out.println("  ERROR: '" + name + "' not found in " + sec.getName() + ".");
    }

    private static void updateItem() {
        Section sec = pickSection("Update item in which section?");
        if (sec == null) return;
        sec.display();

        System.out.print("\n  Item name to update: ");
        String name = sc.nextLine().trim();

        Optional<GroceryItem> opt = sec.findItem(name);
        if (opt.isEmpty()) { System.out.println("  ERROR: Item not found."); return; }

        GroceryItem item = opt.get();
        DateTimeFormatter fmt = DateTimeFormatter.ofPattern("MM/dd/yyyy");
        System.out.println("\n  What would you like to update?");
        System.out.printf ("    1. Price           (current: $%.2f)%n",   item.getPrice());
        System.out.printf ("    2. Count           (current: %d)%n",      item.getCount());
        System.out.printf ("    3. Last stock date (current: %s)%n",      item.getLastStockDate().format(fmt));
        System.out.println("    4. Cancel");

        int choice = readInt("  Choice: ", 1, 4);
        switch (choice) {
            case 1 -> { item.setPrice(readDouble("  New price ($): "));
                        System.out.println("  SUCCESS: Price updated."); }
            case 2 -> { item.setCount(readInt("  New count: ", 0, Integer.MAX_VALUE));
                        System.out.println("  SUCCESS: Count updated."); }
            case 3 -> { item.setLastStockDate(readDate("  New date (MM/DD/YYYY): "));
                        System.out.println("  SUCCESS: Date updated."); }
            case 4 ->   System.out.println("  Cancelled.");
        }
    }

    private static void searchItem() {
        System.out.print("  Item name to search: ");
        String name = sc.nextLine().trim();

        Optional<Object[]> result = store.findAnywhere(name);
        if (result.isEmpty()) {
            System.out.println("  Not found: '" + name + "' is not in any section.");
        } else {
            Section     sec  = (Section)     result.get()[0];
            GroceryItem item = (GroceryItem) result.get()[1];
            System.out.println("  Found in section: " + sec.getName());
            System.out.println(item);
        }
    }

    // ── Input helpers ─────────────────────────────────────────────────────────

    private static Section pickSection(String prompt) {
        List<String> names = store.getSectionNames();
        System.out.println("\n  " + prompt);
        for (int i = 0; i < names.size(); i++)
            System.out.printf("    %d. %s%n", i + 1, names.get(i));
        int cancel = names.size() + 1;
        System.out.println("    " + cancel + ". Cancel");
        int choice = readInt("  Choice: ", 1, cancel);
        if (choice == cancel) return null;
        return store.getSection(names.get(choice - 1)).orElse(null);
    }

    private static int readInt(String prompt, int min, int max) {
        while (true) {
            System.out.print(prompt);
            try {
                int v = Integer.parseInt(sc.nextLine().trim());
                if (v >= min && v <= max) return v;
                System.out.println("  Please enter a value between " + min + " and " + max + ".");
            } catch (NumberFormatException e) {
                System.out.println("  Invalid input — please enter a whole number.");
            }
        }
    }

    private static double readDouble(String prompt) {
        while (true) {
            System.out.print(prompt);
            try {
                double v = Double.parseDouble(sc.nextLine().trim());
                if (v >= 0) return v;
                System.out.println("  Price must be non-negative.");
            } catch (NumberFormatException e) {
                System.out.println("  Invalid input — please enter a number (e.g. 3.99).");
            }
        }
    }

    private static LocalDate readDate(String prompt) {
        DateTimeFormatter fmt = DateTimeFormatter.ofPattern("MM/dd/yyyy");
        while (true) {
            System.out.print(prompt);
            String input = sc.nextLine().trim();
            if (input.isEmpty()) return LocalDate.now();
            try { return LocalDate.parse(input, fmt); }
            catch (Exception e) {
                System.out.println("  Invalid date. Use MM/DD/YYYY or press Enter for today.");
            }
        }
    }
}