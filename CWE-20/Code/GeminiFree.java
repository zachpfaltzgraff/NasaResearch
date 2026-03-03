import java.util.*;
import java.time.LocalDate;

class GroceryItem {
    String name;
    double price;
    int count;
    LocalDate lastStocked;

    public GroceryItem(String name, double price, int count, LocalDate lastStocked) {
        this.name = name;
        this.price = price;
        this.count = count;
        this.lastStocked = lastStocked;
    }

    @Override
    public String toString() {
        return String.format("%-15s | Price: $%-6.2f | Count: %-4d | Last Stocked: %s", 
                              name, price, count, lastStocked);
    }
}

public class GroceryStore {
    private Map<String, List<GroceryItem>> inventory;

    public GroceryStore() {
        inventory = new HashMap<>();
        // Initialize the 5 required sections
        String[] sections = {"Produce", "Deli", "Frozen and Refrigerated", "Boxed Goods", "Other"};
        for (String s : sections) {
            inventory.put(s, new ArrayList<>());
        }
    }

    // Method to add an item
    public void addItem(String section, GroceryItem item) {
        if (inventory.containsKey(section)) {
            inventory.get(section).add(item);
            System.out.println(item.name + " added to " + section + ".");
        } else {
            System.out.println("Section '" + section + "' does not exist.");
        }
    }

    // Method to remove an item by name
    public void removeItem(String section, String itemName) {
        if (inventory.containsKey(section)) {
            boolean removed = inventory.get(section).removeIf(item -> item.name.equalsIgnoreCase(itemName));
            if (removed) {
                System.out.println(itemName + " removed from " + section + ".");
            } else {
                System.out.println(itemName + " not found in " + section + ".");
            }
        }
    }

    public void displayInventory() {
        System.out.println("\n--- CURRENT STORE INVENTORY ---");
        inventory.forEach((section, items) -> {
            System.out.println("\n[" + section + "]");
            if (items.isEmpty()) System.out.println("  (Empty)");
            items.forEach(item -> System.out.println("  " + item));
        });
    }

    public static void main(String[] args) {
        GroceryStore myStore = new GroceryStore();

        // Adding some initial items
        myStore.addItem("Produce", new GroceryItem("Apples", 0.99, 50, LocalDate.now()));
        myStore.addItem("Deli", new GroceryItem("Turkey Breast", 8.50, 10, LocalDate.of(2024, 5, 20)));
        myStore.addItem("Frozen and Refrigerated", new GroceryItem("Ice Cream", 5.99, 15, LocalDate.now()));
        myStore.addItem("Boxed Goods", new GroceryItem("Cereal", 4.25, 24, LocalDate.of(2024, 5, 15)));
        myStore.addItem("Other", new GroceryItem("Batteries", 12.99, 100, LocalDate.of(2024, 1, 10)));

        // Display state
        myStore.displayInventory();

        // Demonstrate removal
        System.out.println("\n--- Updating Inventory ---");
        myStore.removeItem("Produce", "Apples");
        
        // Final display
        myStore.displayInventory();
    }
}