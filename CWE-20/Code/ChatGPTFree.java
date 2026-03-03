import java.util.*;

class Item {
    private String name;
    private double price;
    private int quantity;
    private String lastStockDate;

    public Item(String name, double price, int quantity, String lastStockDate) {
        this.name = name;
        this.price = price;
        this.quantity = quantity;
        this.lastStockDate = lastStockDate;
    }

    public String getName() {
        return name;
    }

    public void updateStock(int amount, String date) {
        quantity += amount;
        lastStockDate = date;
    }

    public void reduceStock(int amount) {
        if (amount <= quantity) {
            quantity -= amount;
        } else {
            System.out.println("Not enough stock available.");
        }
    }

    public String toString() {
        return "Item: " + name +
                " | Price: $" + price +
                " | Quantity: " + quantity +
                " | Last Stock Date: " + lastStockDate;
    }
}

class Section {
    private String sectionName;
    private ArrayList<Item> items;

    public Section(String sectionName) {
        this.sectionName = sectionName;
        this.items = new ArrayList<>();
    }

    public String getSectionName() {
        return sectionName;
    }

    public void addItem(Item item) {
        items.add(item);
    }

    public void removeItem(String itemName) {
        items.removeIf(item -> item.getName().equalsIgnoreCase(itemName));
    }

    public Item findItem(String itemName) {
        for (Item item : items) {
            if (item.getName().equalsIgnoreCase(itemName)) {
                return item;
            }
        }
        return null;
    }

    public void displayItems() {
        if (items.isEmpty()) {
            System.out.println("No items in this section.");
        } else {
            for (Item item : items) {
                System.out.println(item);
            }
        }
    }
}

public class GroceryStore {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        ArrayList<Section> sections = new ArrayList<>();

        sections.add(new Section("Produce"));
        sections.add(new Section("Deli"));
        sections.add(new Section("Frozen and Refrigerated"));
        sections.add(new Section("Boxed Goods"));
        sections.add(new Section("Other"));

        int choice;

        do {
            System.out.println("\n--- Grocery Store Management ---");
            System.out.println("1. View Sections");
            System.out.println("2. View Items in Section");
            System.out.println("3. Add Item");
            System.out.println("4. Remove Item");
            System.out.println("5. Update Stock");
            System.out.println("0. Exit");
            System.out.print("Enter choice: ");

            choice = scanner.nextInt();
            scanner.nextLine(); // consume newline

            switch (choice) {
                case 1:
                    for (int i = 0; i < sections.size(); i++) {
                        System.out.println((i + 1) + ". " + sections.get(i).getSectionName());
                    }
                    break;

                case 2:
                    System.out.print("Enter section number: ");
                    int sectionIndex = scanner.nextInt() - 1;
                    scanner.nextLine();
                    if (sectionIndex >= 0 && sectionIndex < sections.size()) {
                        sections.get(sectionIndex).displayItems();
                    } else {
                        System.out.println("Invalid section.");
                    }
                    break;

                case 3:
                    System.out.print("Enter section number: ");
                    sectionIndex = scanner.nextInt() - 1;
                    scanner.nextLine();

                    if (sectionIndex >= 0 && sectionIndex < sections.size()) {
                        System.out.print("Item name: ");
                        String name = scanner.nextLine();

                        System.out.print("Price: ");
                        double price = scanner.nextDouble();

                        System.out.print("Quantity: ");
                        int quantity = scanner.nextInt();
                        scanner.nextLine();

                        System.out.print("Last stock date (MM/DD/YYYY): ");
                        String date = scanner.nextLine();

                        sections.get(sectionIndex).addItem(
                                new Item(name, price, quantity, date)
                        );
                        System.out.println("Item added successfully.");
                    } else {
                        System.out.println("Invalid section.");
                    }
                    break;

                case 4:
                    System.out.print("Enter section number: ");
                    sectionIndex = scanner.nextInt() - 1;
                    scanner.nextLine();

                    if (sectionIndex >= 0 && sectionIndex < sections.size()) {
                        System.out.print("Enter item name to remove: ");
                        String name = scanner.nextLine();
                        sections.get(sectionIndex).removeItem(name);
                        System.out.println("Item removed if it existed.");
                    } else {
                        System.out.println("Invalid section.");
                    }
                    break;

                case 5:
                    System.out.print("Enter section number: ");
                    sectionIndex = scanner.nextInt() - 1;
                    scanner.nextLine();

                    if (sectionIndex >= 0 && sectionIndex < sections.size()) {
                        System.out.print("Enter item name: ");
                        String name = scanner.nextLine();

                        Item item = sections.get(sectionIndex).findItem(name);

                        if (item != null) {
                            System.out.print("Enter quantity to add: ");
                            int amount = scanner.nextInt();
                            scanner.nextLine();

                            System.out.print("Enter new stock date (MM/DD/YYYY): ");
                            String date = scanner.nextLine();

                            item.updateStock(amount, date);
                            System.out.println("Stock updated.");
                        } else {
                            System.out.println("Item not found.");
                        }
                    } else {
                        System.out.println("Invalid section.");
                    }
                    break;

                case 0:
                    System.out.println("Exiting system...");
                    break;

                default:
                    System.out.println("Invalid option.");
            }

        } while (choice != 0);

        scanner.close();
    }
}