import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.*;

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

    public String getName() {
        return name;
    }

    public double getPrice() {
        return price;
    }

    public void setPrice(double price) {
        this.price = price;
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }

    public LocalDate getLastStockDate() {
        return lastStockDate;
    }

    public void setLastStockDate(LocalDate lastStockDate) {
        this.lastStockDate = lastStockDate;
    }

    @Override
    public String toString() {
        return String.format("%-30s | Price: $%-7.2f | Count: %-5d | Last Stocked: %s",
                name, price, count, lastStockDate);
    }
}

class GrocerySection {
    private String sectionName;
    private List<GroceryItem> items;

    public GrocerySection(String sectionName) {
        this.sectionName = sectionName;
        this.items = new ArrayList<>();
    }

    public String getSectionName() {
        return sectionName;
    }

    public void addItem(GroceryItem item) {
        // Check if item already exists, if so, increase count
        for (GroceryItem existingItem : items) {
            if (existingItem.getName().equalsIgnoreCase(item.getName())) {
                existingItem.setCount(existingItem.getCount() + item.getCount());
                existingItem.setLastStockDate(item.getLastStockDate());
                System.out.println("✓ Updated '" + item.getName() + "'. New count: " + existingItem.getCount());
                return;
            }
        }
        items.add(item);
        System.out.println("✓ Added '" + item.getName() + "' to " + sectionName + ".");
    }

    public void removeItem(String itemName) {
        for (GroceryItem item : items) {
            if (item.getName().equalsIgnoreCase(itemName)) {
                items.remove(item);
                System.out.println("✓ Removed '" + itemName + "' from " + sectionName + ".");
                return;
            }
        }
        System.out.println("✗ Item '" + itemName + "' not found in " + sectionName + ".");
    }

    public void decreaseItemCount(String itemName, int quantity) {
        for (GroceryItem item : items) {
            if (item.getName().equalsIgnoreCase(itemName)) {
                if (item.getCount() >= quantity) {
                    item.setCount(item.getCount() - quantity);
                    System.out.println("✓ Removed " + quantity + " of '" + itemName + "'. Remaining: " + item.getCount());
                } else {
                    System.out.println("✗ Insufficient stock. Available: " + item.getCount());
                }
                return;
            }
        }
        System.out.println("✗ Item '" + itemName + "' not found in " + sectionName + ".");
    }

    public void displayItems() {
        if (items.isEmpty()) {
            System.out.println("No items in this section.");
            return;
        }
        System.out.println("\n" + "=".repeat(100));
        System.out.println("SECTION: " + sectionName);
        System.out.println("=".repeat(100));
        for (int i = 0; i < items.size(); i++) {
            System.out.println((i + 1) + ". " + items.get(i));
        }
        System.out.println("=".repeat(100));
    }

    public GroceryItem getItem(String itemName) {
        for (GroceryItem item : items) {
            if (item.getName().equalsIgnoreCase(itemName)) {
                return item;
            }
        }
        return null;
    }

    public List<GroceryItem> getItems() {
        return items;
    }
}

class GroceryStore {
    private Map<Integer, GrocerySection> sections;
    private Scanner scanner;
    private DateTimeFormatter dateFormatter;

    public GroceryStore() {
        this.sections = new LinkedHashMap<>();
        this.scanner = new Scanner(System.in);
        this.dateFormatter = DateTimeFormatter.ofPattern("yyyy-MM-dd");
        initializeSections();
        initializeSampleData();
    }

    private void initializeSections() {
        sections.put(1, new GrocerySection("Produce"));
        sections.put(2, new GrocerySection("Deli"));
        sections.put(3, new GrocerySection("Frozen and Refrigerated"));
        sections.put(4, new GrocerySection("Boxed Goods"));
        sections.put(5, new GrocerySection("Other"));
    }

    private void initializeSampleData() {
        // Produce
        sections.get(1).addItem(new GroceryItem("Apples", 1.50, 50, LocalDate.of(2026, 3, 1)));
        sections.get(1).addItem(new GroceryItem("Bananas", 0.99, 75, LocalDate.of(2026, 3, 1)));
        sections.get(1).addItem(new GroceryItem("Carrots", 0.75, 40, LocalDate.of(2026, 2, 28)));

        // Deli
        sections.get(2).addItem(new GroceryItem("Roast Beef", 12.99, 20, LocalDate.of(2026, 3, 2)));
        sections.get(2).addItem(new GroceryItem("Turkey Breast", 10.99, 25, LocalDate.of(2026, 3, 2)));
        sections.get(2).addItem(new GroceryItem("Swiss Cheese", 8.99, 15, LocalDate.of(2026, 3, 1)));

        // Frozen and Refrigerated
        sections.get(3).addItem(new GroceryItem("Ice Cream", 5.99, 35, LocalDate.of(2026, 2, 27)));
        sections.get(3).addItem(new GroceryItem("Frozen Pizza", 7.99, 30, LocalDate.of(2026, 2, 26)));
        sections.get(3).addItem(new GroceryItem("Milk", 3.99, 45, LocalDate.of(2026, 3, 2)));

        // Boxed Goods
        sections.get(4).addItem(new GroceryItem("Cereal", 4.49, 60, LocalDate.of(2026, 2, 25)));
        sections.get(4).addItem(new GroceryItem("Pasta", 1.99, 100, LocalDate.of(2026, 2, 20)));
        sections.get(4).addItem(new GroceryItem("Crackers", 3.49, 50, LocalDate.of(2026, 2, 28)));

        // Other
        sections.get(5).addItem(new GroceryItem("Paper Towels", 6.99, 25, LocalDate.of(2026, 2, 15)));
        sections.get(5).addItem(new GroceryItem("Cleaning Spray", 4.99, 40, LocalDate.of(2026, 2, 18)));
        sections.get(5).addItem(new GroceryItem("Soda", 2.49, 80, LocalDate.of(2026, 3, 1)));
    }

    public void displayMenu() {
        System.out.println("\n" + "╔" + "═".repeat(50) + "╗");
        System.out.println("║" + " ".repeat(12) + "🏪 GROCERY STORE SYSTEM 🏪" + " ".repeat(12) + "║");
        System.out.println("╠" + "═".repeat(50) + "╣");
        System.out.println("║ 1. View All Sections                               ║");
        System.out.println("║ 2. View Specific Section                           ║");
        System.out.println("║ 3. Add Item to Section                             ║");
        System.out.println("║ 4. Remove Item from Section                        ║");
        System.out.println("║ 5. Decrease Item Count                             ║");
        System.out.println("║ 6. Search for Item                                 ║");
        System.out.println("║ 7. View Inventory Summary                          ║");
        System.out.println("║ 8. Exit                                            ║");
        System.out.println("╚" + "═".repeat(50) + "╝");
        System.out.print("Enter your choice (1-8): ");
    }

    public void displaySectionMenu() {
        System.out.println("\n📂 SELECT A SECTION:");
        for (Map.Entry<Integer, GrocerySection> entry : sections.entrySet()) {
            System.out.println(entry.getKey() + ". " + entry.getValue().getSectionName());
        }
        System.out.print("Enter section number: ");
    }

    public void viewAllSections() {
        for (GrocerySection section : sections.values()) {
            section.displayItems();
        }
    }

    public void viewSpecificSection() {
        displaySectionMenu();
        int sectionChoice = getIntInput();
        if (sections.containsKey(sectionChoice)) {
            sections.get(sectionChoice).displayItems();
        } else {
            System.out.println("✗ Invalid section number.");
        }
    }

    public void addItemToSection() {
        displaySectionMenu();
        int sectionChoice = getIntInput();
        if (!sections.containsKey(sectionChoice)) {
            System.out.println("✗ Invalid section number.");
            return;
        }

        System.out.print("Enter item name: ");
        String itemName = scanner.nextLine().trim();
        if (itemName.isEmpty()) {
            System.out.println("✗ Item name cannot be empty.");
            return;
        }

        System.out.print("Enter price: $");
        double price = getDoubleInput();
        if (price < 0) {
            System.out.println("✗ Price cannot be negative.");
            return;
        }

        System.out.print("Enter quantity: ");
        int quantity = getIntInput();
        if (quantity <= 0) {
            System.out.println("✗ Quantity must be greater than 0.");
            return;
        }

        System.out.print("Enter stock date (yyyy-MM-dd) [or press Enter for today]: ");
        String dateInput = scanner.nextLine().trim();
        LocalDate stockDate;
        try {
            stockDate = dateInput.isEmpty() ? LocalDate.now() : LocalDate.parse(dateInput, dateFormatter);
        } catch (Exception e) {
            System.out.println("✗ Invalid date format. Using today's date.");
            stockDate = LocalDate.now();
        }

        GroceryItem newItem = new GroceryItem(itemName, price, quantity, stockDate);
        sections.get(sectionChoice).addItem(newItem);
    }

    public void removeItemFromSection() {
        displaySectionMenu();
        int sectionChoice = getIntInput();
        if (!sections.containsKey(sectionChoice)) {
            System.out.println("✗ Invalid section number.");
            return;
        }

        System.out.print("Enter item name to remove: ");
        String itemName = scanner.nextLine().trim();
        sections.get(sectionChoice).removeItem(itemName);
    }

    public void decreaseItemCount() {
        displaySectionMenu();
        int sectionChoice = getIntInput();
        if (!sections.containsKey(sectionChoice)) {
            System.out.println("✗ Invalid section number.");
            return;
        }

        System.out.print("Enter item name: ");
        String itemName = scanner.nextLine().trim();

        System.out.print("Enter quantity to remove: ");
        int quantity = getIntInput();
        if (quantity <= 0) {
            System.out.println("✗ Quantity must be greater than 0.");
            return;
        }

        sections.get(sectionChoice).decreaseItemCount(itemName, quantity);
    }

    public void searchForItem() {
        System.out.print("Enter item name to search: ");
        String searchTerm = scanner.nextLine().trim().toLowerCase();

        boolean found = false;
        for (Map.Entry<Integer, GrocerySection> entry : sections.entrySet()) {
            GrocerySection section = entry.getValue();
            for (GroceryItem item : section.getItems()) {
                if (item.getName().toLowerCase().contains(searchTerm)) {
                    if (!found) {
                        System.out.println("\n🔍 SEARCH RESULTS:");
                        found = true;
                    }
                    System.out.println("[" + section.getSectionName() + "] " + item);
                }
            }
        }

        if (!found) {
            System.out.println("✗ No items found matching '" + searchTerm + "'.");
        } else {
            System.out.println();
        }
    }

    public void viewInventorySummary() {
        System.out.println("\n" + "═".repeat(100));
        System.out.println("INVENTORY SUMMARY");
        System.out.println("═".repeat(100));

        int totalItems = 0;
        double totalValue = 0.0;

        for (Map.Entry<Integer, GrocerySection> entry : sections.entrySet()) {
            GrocerySection section = entry.getValue();
            List<GroceryItem> items = section.getItems();

            int sectionItemCount = 0;
            double sectionValue = 0.0;

            for (GroceryItem item : items) {
                sectionItemCount += item.getCount();
                sectionValue += item.getPrice() * item.getCount();
            }

            totalItems += sectionItemCount;
            totalValue += sectionValue;

            System.out.printf("%-30s | Items: %-6d | Value: $%-10.2f%n",
                    section.getSectionName(), sectionItemCount, sectionValue);
        }

        System.out.println("═".repeat(100));
        System.out.printf("%-30s | Items: %-6d | Value: $%-10.2f%n", "TOTAL", totalItems, totalValue);
        System.out.println("═".repeat(100) + "\n");
    }

    private int getIntInput() {
        try {
            int value = Integer.parseInt(scanner.nextLine().trim());
            return value;
        } catch (NumberFormatException e) {
            System.out.println("✗ Invalid input. Please enter a number.");
            return -1;
        }
    }

    private double getDoubleInput() {
        try {
            double value = Double.parseDouble(scanner.nextLine().trim());
            return value;
        } catch (NumberFormatException e) {
            System.out.println("✗ Invalid input. Please enter a valid price.");
            return -1;
        }
    }

    public void run() {
        boolean running = true;
        System.out.println("\n🏪 Welcome to the Grocery Store Management System!");

        while (running) {
            displayMenu();
            int choice = getIntInput();

            switch (choice) {
                case 1:
                    viewAllSections();
                    break;
                case 2:
                    viewSpecificSection();
                    break;
                case 3:
                    addItemToSection();
                    break;
                case 4:
                    removeItemFromSection();
                    break;
                case 5:
                    decreaseItemCount();
                    break;
                case 6:
                    searchForItem();
                    break;
                case 7:
                    viewInventorySummary();
                    break;
                case 8:
                    System.out.println("\n👋 Thank you for using Grocery Store Management System. Goodbye!");
                    running = false;
                    break;
                default:
                    System.out.println("✗ Invalid choice. Please enter a number between 1 and 8.");
            }
        }
        scanner.close();
    }

    public static void main(String[] args) {
        GroceryStore store = new GroceryStore();
        store.run();
    }
}