#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #define CLEAR_CMD "cls"
#else
    #define CLEAR_CMD "clear"
#endif

#define MAX_NAME 50
#define MAX_PASS 50
#define MAX_ADDR 100
#define MAX_PHONE 15
#define MAX_CATEGORY 30
#define MAX_ORDER_ITEMS 20

/* =============================== DATA STRUCTURES =============================== */

/* 1. SINGLY LINKED LIST - Menu Items by Category */
typedef struct FoodItem {
    int id;
    char name[80];
    char category[30];
    float price;
    int stock;
    struct FoodItem *next;
} FoodItem;

/* 2. DOUBLY LINKED LIST - Order Items */
typedef struct OrderItem {
    int itemId;
    char itemName[80];
    int quantity;
    float price;
    struct OrderItem *prev;
    struct OrderItem *next;
} OrderItem;

/* 3. ORDER DETAILS with Status */
typedef struct Order {
    int orderId;
    char username[MAX_NAME];
    char address[MAX_ADDR];
    char phone[MAX_PHONE];
    OrderItem *items;  /* Doubly linked list of items */
    int itemCount;
    float subtotal;
    float discount;
    float deliveryFee;
    float tax;
    float total;
    int priority; /* 1-Low, 2-Normal, 3-High, 4-Express */
    int status;   /* 0=Pending, 1=Confirmed, 2=Preparing, 3=Out for Delivery, 4=Delivered, 5=Cancelled */
    time_t orderTime;
    time_t statusTime;
    struct Order *next; /* For order stack */
} Order;

/* 4. STACK - Order Processing */
typedef struct OrderStack {
    Order order;
    struct OrderStack *next;
} OrderStack;

/* 5. QUEUE - Delivery Queue */
typedef struct Delivery {
    Order order;
    struct Delivery *next;
} Delivery;

/* 6. BST - User Management */
typedef struct User {
    char username[MAX_NAME];
    char password[MAX_PASS];
    char address[MAX_ADDR];
    char phone[MAX_PHONE];
    int loyaltyPoints;
    struct User *left;
    struct User *right;
} User;

/* 7. AVL Tree - Order History */
typedef struct OrderHistory {
    Order order;
    int height;
    struct OrderHistory *left;
    struct OrderHistory *right;
} OrderHistory;

/* 8. SINGLY LINKED LIST - Promo Codes (Replaced Circular Linked List) */
typedef struct PromoCode {
    char code[20];
    float discount; /* percentage */
    struct PromoCode *next;
} PromoCode;

/* 9. DOUBLY LINKED LIST - Shopping Cart */
typedef struct CartItem {
    int itemId;
    char itemName[80];
    int quantity;
    float price;
    struct CartItem *prev;
    struct CartItem *next;
} CartItem;

/* =============================== GLOBAL VARIABLES =============================== */
FoodItem *menuHead = NULL;           /* Singly Linked List */
CartItem *cartHead = NULL;           /* Doubly Linked List - Shopping Cart */
CartItem *cartTail = NULL;
PromoCode *promoHead = NULL;         /* Singly Linked List */
OrderStack *orderStackTop = NULL;    /* Stack */
Delivery *deliveryFront = NULL;      /* Queue Front */
Delivery *deliveryRear = NULL;       /* Queue Rear */
User *userRoot = NULL;               /* BST Root */
OrderHistory *historyRoot = NULL;    /* AVL Tree Root */

int currentOrderId = 1000;

/* =============================== FUNCTION PROTOTYPES =============================== */
/* Utility Functions */
void clearScreen();
void pressEnter();
void printHeader(const char *title);
void printLine();
char* getStatusText(int status);
char* getPriorityText(int priority);

/* Singly Linked List - Menu */
FoodItem* createFoodItem(int id, const char *name, const char *category, float price, int stock);
void addToMenu(const char *name, const char *category, float price, int stock);
void displayAllMenu();
FoodItem* findMenuItem(int id);
void updateStock(int itemId, int quantity);

/* Doubly Linked List - Shopping Cart */
void addToCart(int itemId, int quantity);
void displayCart();
void removeFromCart(int itemId);
void clearCart();
float calculateCartTotal();

/* Singly Linked List - Promo Codes (Replaced Circular Linked List) */
void addPromoCode(const char *code, float discount);
float applyPromoCode(const char *code, float total);
void displayPromoCodes();

/* Order Management */
Order* createOrder(const char *username, const char *address, const char *phone, int priority);
OrderItem* createOrderItem(int itemId, const char *itemName, int quantity, float price);
void addItemToOrder(Order *order, int itemId, const char *itemName, int quantity, float price);
void displayOrderDetails(Order *order);
void updateOrderStatus(Order *order, int newStatus);

/* Stack - Order Processing */
void pushOrder(Order order);
Order popOrder();
void displayOrderStack();

/* Queue - Delivery System */
void enqueueDelivery(Order order);
Order dequeueDelivery();
void displayDeliveryQueue();

/* BST - User Management */
User* createUser(const char *username, const char *password, const char *address, const char *phone);
User* insertUser(User *root, User *newUser);
User* searchUser(User *root, const char *username);
void displayUsersInorder(User *root);
void addLoyaltyPoints(const char *username, float purchaseAmount);
void saveUsersInorder(User *root, FILE *file);

/* AVL Tree - Order History */
int height(OrderHistory *node);
int maxInt(int a, int b);
OrderHistory* rightRotateAVL(OrderHistory *y);
OrderHistory* leftRotateAVL(OrderHistory *x);
int getBalanceAVL(OrderHistory *node);
OrderHistory* createOrderHistory(Order order);
OrderHistory* insertOrderHistory(OrderHistory *node, Order order);
void displayOrderHistoryInorder(OrderHistory *root);
OrderHistory* searchOrderHistoryById(OrderHistory *root, int orderId);
void displayUserOrderHistory(OrderHistory *root, const char *username);
Order* searchOrderById(int orderId);
void displayOrderStatus(int orderId, const char *username, int isAdmin);

/* File Handling */
void saveData();
void loadData();

/* Core Functions */
void initializeSystem();
void checkout(const char *username, const char *address, const char *phone);
void userDashboard(const char *username);
void adminDashboard();
void userLogin();
void userSignup();
void adminLogin();

/* =============================== UTILITY FUNCTIONS =============================== */
void clearScreen() {
    system(CLEAR_CMD);
}

void pressEnter() {
    printf("\nPress Enter to continue...");
    while(getchar() != '\n');
    getchar();
}

void printHeader(const char *title) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║ %-58s ║\n", title);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void printLine() {
    printf("────────────────────────────────────────────────────────────────\n");
}

char* getStatusText(int status) {
    switch(status) {
        case 0: return "📝 Pending";
        case 1: return "✅ Confirmed";
        case 2: return "👨‍🍳 Preparing";
        case 3: return "🚚 Out for Delivery";
        case 4: return "🎉 Delivered";
        case 5: return "❌ Cancelled";
        default: return "Unknown";
    }
}

char* getPriorityText(int priority) {
    switch(priority) {
        case 1: return "🐢 Low (4-6 hours)";
        case 2: return "🚶 Normal (2-4 hours)";
        case 3: return "⚡ High (1-2 hours)";
        case 4: return "🚀 Express (30-60 min)";
        default: return "Unknown";
    }
}

/* =============================== SINGLY LINKED LIST - MENU =============================== */
FoodItem* createFoodItem(int id, const char *name, const char *category, float price, int stock) {
    FoodItem *newItem = (FoodItem*)malloc(sizeof(FoodItem));
    newItem->id = id;
    strcpy(newItem->name, name);
    strcpy(newItem->category, category);
    newItem->price = price;
    newItem->stock = stock;
    newItem->next = NULL;
    return newItem;
}

void addToMenu(const char *name, const char *category, float price, int stock) {
    static int nextId = 1;
    FoodItem *newItem = createFoodItem(nextId++, name, category, price, stock);
    
    if (menuHead == NULL) {
        menuHead = newItem;
    } else {
        FoodItem *current = menuHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newItem;
    }
    printf("✓ Added: %s ($%.2f) to %s category\n", name, price, category);
}

void displayAllMenu() {
    printHeader("MENU - ALL ITEMS");
    
    FoodItem *current = menuHead;
    char currentCategory[30] = "";
    int firstCategory = 1;
    
    while (current != NULL) {
        if (strcmp(currentCategory, current->category) != 0) {
            if (!firstCategory) {
                printf("\n");
            }
            strcpy(currentCategory, current->category);
            printf("\n【 %s 】\n", currentCategory);
            printf("ID\tName\t\t\tPrice\tStock\n");
            printf("────────────────────────────────────────────────\n");
            firstCategory = 0;
        }
        printf("%d\t%-20s\t$%.2f\t%d\n", 
               current->id, current->name, current->price, current->stock);
        current = current->next;
    }
}

FoodItem* findMenuItem(int id) {
    FoodItem *current = menuHead;
    while (current != NULL) {
        if (current->id == id) return current;
        current = current->next;
    }
    return NULL;
}

void updateStock(int itemId, int quantity) {
    FoodItem *item = findMenuItem(itemId);
    if (item != NULL) {
        item->stock -= quantity;
        if (item->stock < 0) item->stock = 0;
    }
}

/* =============================== DOUBLY LINKED LIST - SHOPPING CART =============================== */
void addToCart(int itemId, int quantity) {
    FoodItem *item = findMenuItem(itemId);
    if (item == NULL) {
        printf("Item not found!\n");
        return;
    }
    
    if (item->stock < quantity) {
        printf("Insufficient stock! Only %d available.\n", item->stock);
        return;
    }
    
    CartItem *newItem = (CartItem*)malloc(sizeof(CartItem));
    newItem->itemId = itemId;
    strcpy(newItem->itemName, item->name);
    newItem->quantity = quantity;
    newItem->price = item->price;
    newItem->prev = NULL;
    newItem->next = NULL;
    
    if (cartHead == NULL) {
        cartHead = cartTail = newItem;
    } else {
        cartTail->next = newItem;
        newItem->prev = cartTail;
        cartTail = newItem;
    }
    
    printf("✓ Added %d x %s to cart\n", quantity, item->name);
}

void displayCart() {
    if (cartHead == NULL) {
        printf("Your cart is empty!\n");
        return;
    }
    
    printHeader("SHOPPING CART");
    printf("Item\t\t\tQuantity\tPrice\tSubtotal\n");
    printf("────────────────────────────────────────────────────────────\n");
    
    CartItem *current = cartHead;
    float total = 0;
    int itemCount = 0;
    
    while (current != NULL) {
        float subtotal = current->price * current->quantity;
        printf("%-20s\t%d\t\t$%.2f\t$%.2f\n", 
               current->itemName, current->quantity, current->price, subtotal);
        total += subtotal;
        itemCount += current->quantity;
        current = current->next;
    }
    
    printf("────────────────────────────────────────────────────────────\n");
    printf("Total Items: %d\t\t\t\tTotal: $%.2f\n", itemCount, total);
}

void removeFromCart(int itemId) {
    CartItem *current = cartHead;
    
    while (current != NULL) {
        if (current->itemId == itemId) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                cartHead = current->next;
            }
            
            if (current->next != NULL) {
                current->next->prev = current->prev;
            } else {
                cartTail = current->prev;
            }
            
            printf("Removed %s from cart\n", current->itemName);
            free(current);
            return;
        }
        current = current->next;
    }
    
    printf("Item not found in cart!\n");
}

void clearCart() {
    CartItem *current = cartHead;
    CartItem *next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    cartHead = cartTail = NULL;
    printf("Cart cleared!\n");
}

float calculateCartTotal() {
    CartItem *current = cartHead;
    float total = 0;
    
    while (current != NULL) {
        total += current->price * current->quantity;
        current = current->next;
    }
    
    return total;
}

/* =============================== SINGLY LINKED LIST - PROMO CODES =============================== */
void addPromoCode(const char *code, float discount) {
    PromoCode *newCode = (PromoCode*)malloc(sizeof(PromoCode));
    strcpy(newCode->code, code);
    newCode->discount = discount;
    newCode->next = NULL;
    
    if (promoHead == NULL) {
        promoHead = newCode;
    } else {
        PromoCode *current = promoHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newCode;
    }
    
    printf("✓ Promo code %s added (%.0f%% discount)\n", code, discount);
}

float applyPromoCode(const char *code, float total) {
    if (promoHead == NULL) {
        printf("No promo codes available!\n");
        return total;
    }
    
    PromoCode *current = promoHead;
    while (current != NULL) {
        if (strcmp(current->code, code) == 0) {
            float discount = total * (current->discount / 100);
            float newTotal = total - discount;
            printf("✓ Applied promo code %s: %.0f%% discount (-$%.2f)\n", 
                   code, current->discount, discount);
            return newTotal;
        }
        current = current->next;
    }
    
    printf("Invalid promo code!\n");
    return total;
}

void displayPromoCodes() {
    if (promoHead == NULL) {
        printf("No promo codes available!\n");
        return;
    }
    
    printHeader("AVAILABLE PROMO CODES");
    printf("Code\t\tDiscount\n");
    printf("────────────────────────\n");
    
    PromoCode *current = promoHead;
    while (current != NULL) {
        printf("%-10s\t%.0f%%\n", current->code, current->discount);
        current = current->next;
    }
}

/* =============================== ORDER MANAGEMENT =============================== */
Order* createOrder(const char *username, const char *address, const char *phone, int priority) {
    Order *newOrder = (Order*)malloc(sizeof(Order));
    newOrder->orderId = currentOrderId++;
    strcpy(newOrder->username, username);
    strcpy(newOrder->address, address);
    strcpy(newOrder->phone, phone);
    newOrder->items = NULL;
    newOrder->itemCount = 0;
    newOrder->subtotal = 0;
    newOrder->discount = 0;
    newOrder->deliveryFee = 2.99;
    newOrder->tax = 0;
    newOrder->total = 0;
    newOrder->priority = priority;
    newOrder->status = 0; /* Pending */
    newOrder->orderTime = time(NULL);
    newOrder->statusTime = time(NULL);
    newOrder->next = NULL;
    
    return newOrder;
}

OrderItem* createOrderItem(int itemId, const char *itemName, int quantity, float price) {
    OrderItem *newItem = (OrderItem*)malloc(sizeof(OrderItem));
    newItem->itemId = itemId;
    strcpy(newItem->itemName, itemName);
    newItem->quantity = quantity;
    newItem->price = price;
    newItem->prev = NULL;
    newItem->next = NULL;
    
    return newItem;
}

void addItemToOrder(Order *order, int itemId, const char *itemName, int quantity, float price) {
    OrderItem *newItem = createOrderItem(itemId, itemName, quantity, price);
    
    if (order->items == NULL) {
        order->items = newItem;
    } else {
        OrderItem *current = order->items;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newItem;
        newItem->prev = current;
    }
    
    order->itemCount++;
    order->subtotal += price * quantity;
}

void displayOrderDetails(Order *order) {
    printf("\n════════════════════════════════════════════════════════════\n");
    printf("                    ORDER DETAILS\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Order ID: #%d\n", order->orderId);
    printf("Customer: %s\n", order->username);
    printf("Address: %s\n", order->address);
    printf("Phone: %s\n", order->phone);
    printf("Order Time: %s", ctime(&order->orderTime));
    printf("Status: %s (Updated: %s)", getStatusText(order->status), ctime(&order->statusTime));
    printf("Priority: %s\n", getPriorityText(order->priority));
    printf("\n────────────────────────────────────────────────────────────\n");
    printf("ORDER ITEMS:\n");
    printf("────────────────────────────────────────────────────────────\n");
    
    if (order->items == NULL) {
        printf("No items in order.\n");
    } else {
        OrderItem *current = order->items;
        int itemNum = 1;
        
        printf("No.\tItem\t\t\tQty\tPrice\tSubtotal\n");
        printf("────────────────────────────────────────────────────────────\n");
        
        while (current != NULL) {
            float subtotal = current->price * current->quantity;
            printf("%d.\t%-20s\t%d\t$%.2f\t$%.2f\n", 
                   itemNum++, current->itemName, current->quantity, current->price, subtotal);
            current = current->next;
        }
    }
    
    printf("\n────────────────────────────────────────────────────────────\n");
    printf("ORDER SUMMARY:\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("Subtotal: $%.2f\n", order->subtotal);
    printf("Discount: -$%.2f\n", order->discount);
    printf("Delivery Fee: $%.2f\n", order->deliveryFee);
    printf("Tax (8%%): $%.2f\n", order->tax);
    printf("────────────────────────────────────────────────────────────\n");
    printf("TOTAL: $%.2f\n", order->total);
    printf("════════════════════════════════════════════════════════════\n");
}

void updateOrderStatus(Order *order, int newStatus) {
    order->status = newStatus;
    order->statusTime = time(NULL);
}

/* =============================== STACK - ORDER PROCESSING =============================== */
void pushOrder(Order order) {
    OrderStack *newOrder = (OrderStack*)malloc(sizeof(OrderStack));
    newOrder->order = order;
    newOrder->next = orderStackTop;
    orderStackTop = newOrder;
    
    printf("✓ Order #%d placed successfully!\n", order.orderId);
}

Order popOrder() {
    Order emptyOrder = {0};
    
    if (orderStackTop == NULL) {
        printf("No orders to process!\n");
        return emptyOrder;
    }
    
    OrderStack *temp = orderStackTop;
    Order order = temp->order;
    orderStackTop = orderStackTop->next;
    
    free(temp);
    return order;
}

void displayOrderStack() {
    if (orderStackTop == NULL) {
        printf("No pending orders!\n");
        return;
    }
    
    printHeader("PENDING ORDERS (STACK)");
    printf("Order ID\tCustomer\t\tStatus\t\t\tTotal\tTime\n");
    printf("─────────────────────────────────────────────────────────────────────────────────────────────\n");
    
    OrderStack *current = orderStackTop;
    while (current != NULL) {
        printf("#%d\t\t%-15s\t%-20s\t$%.2f\t%s", 
               current->order.orderId, current->order.username, 
               getStatusText(current->order.status), current->order.total, 
               ctime(&current->order.orderTime));
        current = current->next;
    }
}

/* =============================== QUEUE - DELIVERY SYSTEM =============================== */
void enqueueDelivery(Order order) {
    Delivery *newDelivery = (Delivery*)malloc(sizeof(Delivery));
    newDelivery->order = order;
    newDelivery->next = NULL;
    
    if (deliveryRear == NULL) {
        deliveryFront = deliveryRear = newDelivery;
    } else {
        /* Priority-based insertion */
        if (order.priority > deliveryFront->order.priority) {
            /* Insert at front for highest priority */
            newDelivery->next = deliveryFront;
            deliveryFront = newDelivery;
        } else {
            /* Find correct position */
            Delivery *current = deliveryFront;
            Delivery *prev = NULL;
            
            while (current != NULL && current->order.priority >= order.priority) {
                prev = current;
                current = current->next;
            }
            
            if (prev == NULL) {
                newDelivery->next = deliveryFront;
                deliveryFront = newDelivery;
            } else {
                newDelivery->next = current;
                prev->next = newDelivery;
                
                if (newDelivery->next == NULL) {
                    deliveryRear = newDelivery;
                }
            }
        }
    }
    
    printf("✓ Delivery queued for Order #%d\n", order.orderId);
}

Order dequeueDelivery() {
    Order emptyOrder = {0};
    
    if (deliveryFront == NULL) {
        printf("No deliveries pending!\n");
        return emptyOrder;
    }
    
    Delivery *temp = deliveryFront;
    Order order = temp->order;
    deliveryFront = deliveryFront->next;
    
    if (deliveryFront == NULL) {
        deliveryRear = NULL;
    }
    
    free(temp);
    return order;
}

void displayDeliveryQueue() {
    if (deliveryFront == NULL) {
        printf("No deliveries in queue!\n");
        return;
    }
    
    printHeader("DELIVERY QUEUE");
    printf("Position\tOrder ID\tCustomer\t\tStatus\t\t\tPriority\n");
    printf("─────────────────────────────────────────────────────────────────────────────────────────────\n");
    
    Delivery *current = deliveryFront;
    int position = 1;
    
    while (current != NULL) {
        printf("%d\t\t#%d\t\t%-15s\t%-20s\t%s\n", 
               position++, current->order.orderId, current->order.username,
               getStatusText(current->order.status), getPriorityText(current->order.priority));
        current = current->next;
    }
}

/* =============================== BST - USER MANAGEMENT =============================== */
User* createUser(const char *username, const char *password, const char *address, const char *phone) {
    User *newUser = (User*)malloc(sizeof(User));
    strcpy(newUser->username, username);
    strcpy(newUser->password, password);
    strcpy(newUser->address, address);
    strcpy(newUser->phone, phone);
    newUser->loyaltyPoints = 0;
    newUser->left = NULL;
    newUser->right = NULL;
    return newUser;
}

User* insertUser(User *root, User *newUser) {
    if (root == NULL) {
        return newUser;
    }
    
    int cmp = strcmp(newUser->username, root->username);
    
    if (cmp < 0) {
        root->left = insertUser(root->left, newUser);
    } else if (cmp > 0) {
        root->right = insertUser(root->right, newUser);
    } else {
        printf("✗ Username already exists!\n");
        free(newUser);
        return root;
    }
    
    return root;
}

User* searchUser(User *root, const char *username) {
    if (root == NULL || strcmp(root->username, username) == 0) {
        return root;
    }
    
    if (strcmp(username, root->username) < 0) {
        return searchUser(root->left, username);
    }
    
    return searchUser(root->right, username);
}

void displayUsersInorder(User *root) {
    if (root != NULL) {
        displayUsersInorder(root->left);
        printf("%-15s\t%-30s\t%s\t%d points\n", 
               root->username, root->address, root->phone, root->loyaltyPoints);
        displayUsersInorder(root->right);
    }
}

void addLoyaltyPoints(const char *username, float purchaseAmount) {
    User *user = searchUser(userRoot, username);
    if (user != NULL) {
        int points = (int)(purchaseAmount * 10);
        user->loyaltyPoints += points;
        printf("✓ Added %d loyalty points to %s\n", points, username);
    }
}

/* =============================== AVL TREE - ORDER HISTORY =============================== */
int height(OrderHistory *node) {
    return node ? node->height : 0;
}

int maxInt(int a, int b) {
    return (a > b) ? a : b;
}

OrderHistory* rightRotateAVL(OrderHistory *y) {
    OrderHistory *x = y->left;
    OrderHistory *T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = maxInt(height(y->left), height(y->right)) + 1;
    x->height = maxInt(height(x->left), height(x->right)) + 1;
    
    return x;
}

OrderHistory* leftRotateAVL(OrderHistory *x) {
    OrderHistory *y = x->right;
    OrderHistory *T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = maxInt(height(x->left), height(x->right)) + 1;
    y->height = maxInt(height(y->left), height(y->right)) + 1;
    
    return y;
}

int getBalanceAVL(OrderHistory *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

OrderHistory* createOrderHistory(Order order) {
    OrderHistory *newNode = (OrderHistory*)malloc(sizeof(OrderHistory));
    newNode->order = order;
    newNode->height = 1;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

OrderHistory* insertOrderHistory(OrderHistory *node, Order order) {
    if (node == NULL) {
        return createOrderHistory(order);
    }
    
    if (order.orderId < node->order.orderId) {
        node->left = insertOrderHistory(node->left, order);
    } else if (order.orderId > node->order.orderId) {
        node->right = insertOrderHistory(node->right, order);
    } else {
        return node;
    }
    
    node->height = 1 + maxInt(height(node->left), height(node->right));
    
    int balance = getBalanceAVL(node);
    
    if (balance > 1 && order.orderId < node->left->order.orderId) {
        return rightRotateAVL(node);
    }
    
    if (balance < -1 && order.orderId > node->right->order.orderId) {
        return leftRotateAVL(node);
    }
    
    if (balance > 1 && order.orderId > node->left->order.orderId) {
        node->left = leftRotateAVL(node->left);
        return rightRotateAVL(node);
    }
    
    if (balance < -1 && order.orderId < node->right->order.orderId) {
        node->right = rightRotateAVL(node->right);
        return leftRotateAVL(node);
    }
    
    return node;
}

void displayOrderHistoryInorder(OrderHistory *root) {
    if (root != NULL) {
        displayOrderHistoryInorder(root->left);
        printf("#%d\t\t%s\t\t%s\t\t$%.2f\t%s", 
               root->order.orderId, root->order.username, 
               getStatusText(root->order.status), root->order.total, 
               ctime(&root->order.orderTime));
        displayOrderHistoryInorder(root->right);
    }
}

OrderHistory* searchOrderHistoryById(OrderHistory *root, int orderId) {
    if (root == NULL || root->order.orderId == orderId) {
        return root;
    }
    
    if (orderId < root->order.orderId) {
        return searchOrderHistoryById(root->left, orderId);
    }
    
    return searchOrderHistoryById(root->right, orderId);
}

void displayUserOrderHistory(OrderHistory *root, const char *username) {
    if (root != NULL) {
        displayUserOrderHistory(root->left, username);
        if (strcmp(root->order.username, username) == 0) {
            printf("#%d\t\t%s\t\t$%.2f\t%s", 
                   root->order.orderId, getStatusText(root->order.status), 
                   root->order.total, ctime(&root->order.orderTime));
        }
        displayUserOrderHistory(root->right, username);
    }
}

/* =============================== ORDER TRACKING FUNCTIONS =============================== */
Order* searchOrderById(int orderId) {
    /* Search in order stack */
    OrderStack *stackCurrent = orderStackTop;
    while (stackCurrent != NULL) {
        if (stackCurrent->order.orderId == orderId) {
            return &(stackCurrent->order);
        }
        stackCurrent = stackCurrent->next;
    }
    
    /* Search in delivery queue */
    Delivery *queueCurrent = deliveryFront;
    while (queueCurrent != NULL) {
        if (queueCurrent->order.orderId == orderId) {
            return &(queueCurrent->order);
        }
        queueCurrent = queueCurrent->next;
    }
    
    /* Search in history (AVL tree) */
    OrderHistory *historyNode = searchOrderHistoryById(historyRoot, orderId);
    if (historyNode != NULL) {
        return &(historyNode->order);
    }
    
    return NULL;
}

void displayOrderStatus(int orderId, const char *username, int isAdmin) {
    printHeader("ORDER TRACKING");
    
    Order *order = searchOrderById(orderId);
    if (order == NULL) {
        printf("Order #%d not found!\n", orderId);
        return;
    }
    
    /* Check if user is authorized to view this order */
    if (!isAdmin && strcmp(order->username, username) != 0) {
        printf("Access denied! You can only view your own orders.\n");
        return;
    }
    
    displayOrderDetails(order);
    
    /* Show delivery progress */
    printf("\nDELIVERY PROGRESS:\n");
    printf("[");
    for (int i = 0; i <= 4; i++) {
        if (order->status >= i) {
            printf("█");
        } else {
            printf("░");
        }
    }
    printf("]\n");
    
    /* Show status timeline */
    printf("\nSTATUS TIMELINE:\n");
    printf("1. Order Placed: %s", order->status >= 0 ? "✓ Completed\n" : "○ Pending\n");
    printf("2. Order Confirmed: %s", order->status >= 1 ? "✓ Completed\n" : "○ Pending\n");
    printf("3. Food Preparation: %s", order->status >= 2 ? "✓ Completed\n" : "○ Pending\n");
    printf("4. Out for Delivery: %s", order->status >= 3 ? "✓ Completed\n" : "○ Pending\n");
    printf("5. Order Delivered: %s", order->status >= 4 ? "✓ Completed\n" : "○ Pending\n");
    
    /* Estimated delivery time */
    printf("\nESTIMATED DELIVERY TIME:\n");
    time_t estimatedTime = order->orderTime;
    switch(order->priority) {
        case 1: estimatedTime += 4 * 3600; break;  /* 4 hours */
        case 2: estimatedTime += 2 * 3600; break;  /* 2 hours */
        case 3: estimatedTime += 1 * 3600; break;  /* 1 hour */
        case 4: estimatedTime += 1800; break;      /* 30 minutes */
    }
    printf("Expected by: %s", ctime(&estimatedTime));
}

/* =============================== FILE HANDLING =============================== */
void saveUsersInorder(User *root, FILE *file) {
    if (root != NULL) {
        saveUsersInorder(root->left, file);
        fprintf(file, "%s,%s,%s,%s,%d\n",
                root->username, root->password, root->address,
                root->phone, root->loyaltyPoints);
        saveUsersInorder(root->right, file);
    }
}

void saveData() {
    /* Save Menu */
    FILE *menuFile = fopen("menu.dat", "w");
    FoodItem *menuCurrent = menuHead;
    while (menuCurrent != NULL) {
        fprintf(menuFile, "%d,%s,%s,%.2f,%d\n", 
                menuCurrent->id, menuCurrent->name, menuCurrent->category,
                menuCurrent->price, menuCurrent->stock);
        menuCurrent = menuCurrent->next;
    }
    fclose(menuFile);
    
    /* Save Users */
    FILE *userFile = fopen("users.dat", "w");
    saveUsersInorder(userRoot, userFile);
    fclose(userFile);
    
    /* Save Promo Codes */
    FILE *promoFile = fopen("promo.dat", "w");
    PromoCode *promoCurrent = promoHead;
    while (promoCurrent != NULL) {
        fprintf(promoFile, "%s,%.2f\n", promoCurrent->code, promoCurrent->discount);
        promoCurrent = promoCurrent->next;
    }
    fclose(promoFile);
    
    printf("✓ All data saved successfully!\n");
}

void loadData() {
    /* Load Menu */
    FILE *menuFile = fopen("menu.dat", "r");
    if (menuFile) {
        int id, stock;
        char name[80], category[30];
        float price;
        while (fscanf(menuFile, "%d,%[^,],%[^,],%f,%d\n", &id, name, category, &price, &stock) == 5) {
            addToMenu(name, category, price, stock);
        }
        fclose(menuFile);
    }
    
    /* Load Users */
    FILE *userFile = fopen("users.dat", "r");
    if (userFile) {
        char username[MAX_NAME], password[MAX_PASS], address[MAX_ADDR], phone[MAX_PHONE];
        int loyaltyPoints;
        while (fscanf(userFile, "%[^,],%[^,],%[^,],%[^,],%d\n", 
                     username, password, address, phone, &loyaltyPoints) == 5) {
            User *newUser = createUser(username, password, address, phone);
            newUser->loyaltyPoints = loyaltyPoints;
            userRoot = insertUser(userRoot, newUser);
        }
        fclose(userFile);
    }
    
    /* Load Promo Codes */
    FILE *promoFile = fopen("promo.dat", "r");
    if (promoFile) {
        char code[20];
        float discount;
        while (fscanf(promoFile, "%[^,],%f\n", code, &discount) == 2) {
            addPromoCode(code, discount);
        }
        fclose(promoFile);
    }
    
    /* Load Default Users if none */
    if (userRoot == NULL) {
        User *admin = createUser("admin", "admin123", "Admin Office", "1234567890");
        admin->loyaltyPoints = 1000;
        userRoot = insertUser(userRoot, admin);
        
        User *user = createUser("user", "user123", "123 Main St", "9876543210");
        userRoot = insertUser(userRoot, user);
        
        /* Add some promo codes */
        addPromoCode("WELCOME10", 10);
        addPromoCode("SAVE20", 20);
        addPromoCode("FIRSTORDER", 15);
    }
}

/* =============================== CORE FUNCTIONS =============================== */
void initializeSystem() {
    printf("Initializing Food Delivery System...\n");
    
    /* Load existing data */
    loadData();
    
    /* Add sample menu items if empty */
    if (menuHead == NULL) {
        addToMenu("Margherita Pizza", "Pizza", 12.99, 50);
        addToMenu("Pepperoni Pizza", "Pizza", 14.99, 40);
        addToMenu("Veg Supreme Pizza", "Pizza", 13.99, 30);
        
        addToMenu("Classic Burger", "Burgers", 8.99, 60);
        addToMenu("Cheese Burger", "Burgers", 9.99, 50);
        addToMenu("Chicken Burger", "Burgers", 10.99, 45);
        
        addToMenu("French Fries", "Sides", 3.99, 100);
        addToMenu("Onion Rings", "Sides", 4.99, 80);
        addToMenu("Garlic Bread", "Sides", 2.99, 90);
        
        addToMenu("Coca Cola", "Drinks", 1.99, 200);
        addToMenu("Orange Juice", "Drinks", 2.99, 150);
        addToMenu("Iced Tea", "Drinks", 2.49, 120);
    }
    
    printf("✓ System initialized successfully!\n");
}

void checkout(const char *username, const char *address, const char *phone) {
    if (cartHead == NULL) {
        printf("Your cart is empty! Add items first.\n");
        return;
    }
    
    printHeader("CHECKOUT");
    
    /* Calculate subtotal from cart */
    float subtotal = calculateCartTotal();
    printf("Subtotal: $%.2f\n", subtotal);
    
    /* Apply promo code */
    char promoCode[20];
    printf("Enter promo code (or 'skip'): ");
    scanf("%s", promoCode);
    
    float discount = 0;
    float total = subtotal;
    if (strcmp(promoCode, "skip") != 0) {
        total = applyPromoCode(promoCode, subtotal);
        discount = subtotal - total;
    }
    
    /* Get delivery priority */
    int priority;
    printf("\nSelect delivery priority:\n");
    printf("1. Low (4-6 hours)\n");
    printf("2. Normal (2-4 hours)\n");
    printf("3. High (1-2 hours)\n");
    printf("4. Express (30-60 minutes)\n");
    printf("Choice: ");
    scanf("%d", &priority);
    
    /* Create order */
    Order newOrder;
    newOrder.orderId = currentOrderId++;
    strcpy(newOrder.username, username);
    strcpy(newOrder.address, address);
    strcpy(newOrder.phone, phone);
    newOrder.items = NULL;
    newOrder.itemCount = 0;
    newOrder.subtotal = subtotal;
    newOrder.discount = discount;
    newOrder.deliveryFee = 2.99;
    newOrder.tax = (total + 2.99) * 0.08;
    newOrder.total = total + 2.99 + newOrder.tax;
    newOrder.priority = priority;
    newOrder.status = 0; /* Pending */
    newOrder.orderTime = time(NULL);
    newOrder.statusTime = time(NULL);
    newOrder.next = NULL;
    
    /* Add cart items to order */
    CartItem *cartCurrent = cartHead;
    while (cartCurrent != NULL) {
        addItemToOrder(&newOrder, cartCurrent->itemId, cartCurrent->itemName, 
                       cartCurrent->quantity, cartCurrent->price);
        updateStock(cartCurrent->itemId, cartCurrent->quantity);
        cartCurrent = cartCurrent->next;
    }
    
    /* Push to order stack */
    pushOrder(newOrder);
    
    /* Add to delivery queue */
    enqueueDelivery(newOrder);
    
    /* Add to order history (AVL tree) */
    historyRoot = insertOrderHistory(historyRoot, newOrder);
    
    /* Update loyalty points */
    addLoyaltyPoints(username, newOrder.total);
    
    /* Clear cart */
    clearCart();
    
    printf("\n✓ Order #%d confirmed!\n", newOrder.orderId);
    printf("\nOrder Summary:\n");
    printf("────────────────────────────────────────────────────────────\n");
    displayOrderDetails(&newOrder);
    printf("────────────────────────────────────────────────────────────\n");
}

void userDashboard(const char *username) {
    User *user = searchUser(userRoot, username);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    int choice;
    do {
        clearScreen();
        printHeader("USER DASHBOARD");
        printf("Welcome, %s!\n", username);
        printf("Loyalty Points: %d\n", user->loyaltyPoints);
        printLine();
        
        printf("1. Browse Menu\n");
        printf("2. View Cart\n");
        printf("3. Add to Cart\n");
        printf("4. Remove from Cart\n");
        printf("5. Checkout\n");
        printf("6. View Order History\n");
        printf("7. Track Order Status\n");
        printf("8. View Promo Codes\n");
        printf("9. Logout\n");
        printLine();
        printf("Choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: {
                clearScreen();
                displayAllMenu();
                pressEnter();
                break;
            }
            case 2: {
                clearScreen();
                displayCart();
                pressEnter();
                break;
            }
            case 3: {
                clearScreen();
                displayAllMenu();
                printf("\nEnter item ID to add: ");
                int itemId, quantity;
                scanf("%d", &itemId);
                printf("Enter quantity: ");
                scanf("%d", &quantity);
                addToCart(itemId, quantity);
                pressEnter();
                break;
            }
            case 4: {
                clearScreen();
                displayCart();
                if (cartHead != NULL) {
                    printf("\nEnter item ID to remove: ");
                    int itemId;
                    scanf("%d", &itemId);
                    removeFromCart(itemId);
                }
                pressEnter();
                break;
            }
            case 5: {
                clearScreen();
                checkout(username, user->address, user->phone);
                pressEnter();
                break;
            }
            case 6: {
                clearScreen();
                printHeader("YOUR ORDER HISTORY");
                if (historyRoot == NULL) {
                    printf("No order history yet.\n");
                } else {
                    printf("Order ID\tStatus\t\t\tTotal\t\tOrder Time\n");
                    printf("────────────────────────────────────────────────────────────────\n");
                    displayUserOrderHistory(historyRoot, username);
                }
                pressEnter();
                break;
            }
            case 7: {
                clearScreen();
                printHeader("TRACK ORDER");
                printf("Enter Order ID to track: ");
                int orderId;
                scanf("%d", &orderId);
                displayOrderStatus(orderId, username, 0);
                pressEnter();
                break;
            }
            case 8: {
                clearScreen();
                displayPromoCodes();
                pressEnter();
                break;
            }
            case 9: {
                printf("Logging out...\n");
                break;
            }
            default: {
                printf("Invalid choice!\n");
                pressEnter();
            }
        }
    } while (choice != 9);
}

void adminDashboard() {
    int choice;
    do {
        clearScreen();
        printHeader("ADMIN DASHBOARD");
        
        printf("1. Manage Menu Items\n");
        printf("2. View Pending Orders\n");
        printf("3. Process Next Order\n");
        printf("4. Update Order Status\n");
        printf("5. Manage Deliveries\n");
        printf("6. View All Users\n");
        printf("7. View Order History\n");
        printf("8. Track Specific Order\n");
        printf("9. Add Promo Code\n");
        printf("10. Save All Data\n");
        printf("11. Logout\n");
        printLine();
        printf("Choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: {
                clearScreen();
                printf("1. Add New Item\n");
                printf("2. View All Items\n");
                printf("Choice: ");
                int subChoice;
                scanf("%d", &subChoice);
                
                if (subChoice == 1) {
                    char name[80], category[30];
                    float price;
                    int stock;
                    printf("Item name: ");
                    scanf(" %[^\n]", name);
                    printf("Category: ");
                    scanf(" %[^\n]", category);
                    printf("Price: ");
                    scanf("%f", &price);
                    printf("Stock: ");
                    scanf("%d", &stock);
                    addToMenu(name, category, price, stock);
                } else if (subChoice == 2) {
                    displayAllMenu();
                }
                pressEnter();
                break;
            }
            case 2: {
                clearScreen();
                displayOrderStack();
                pressEnter();
                break;
            }
            case 3: {
                clearScreen();
                Order processed = popOrder();
                if (processed.orderId != 0) {
                    printf("Processing Order #%d...\n", processed.orderId);
                    updateOrderStatus(&processed, 1); /* Confirmed */
                    printf("✓ Order #%d confirmed and ready for preparation!\n", processed.orderId);
                }
                pressEnter();
                break;
            }
            case 4: {
                clearScreen();
                printf("Enter Order ID to update: ");
                int orderId;
                scanf("%d", &orderId);
                
                Order *order = searchOrderById(orderId);
                if (order == NULL) {
                    printf("Order #%d not found!\n", orderId);
                } else {
                    printf("\nCurrent Status: %s\n", getStatusText(order->status));
                    printf("\nSelect new status:\n");
                    printf("0. Pending\n");
                    printf("1. Confirmed\n");
                    printf("2. Preparing\n");
                    printf("3. Out for Delivery\n");
                    printf("4. Delivered\n");
                    printf("5. Cancelled\n");
                    printf("Choice: ");
                    int newStatus;
                    scanf("%d", &newStatus);
                    
                    updateOrderStatus(order, newStatus);
                    printf("✓ Order #%d status updated to: %s\n", orderId, getStatusText(newStatus));
                }
                pressEnter();
                break;
            }
            case 5: {
                clearScreen();
                displayDeliveryQueue();
                pressEnter();
                break;
            }
            case 6: {
                clearScreen();
                printHeader("ALL REGISTERED USERS");
                printf("Username\tAddress\t\t\t\tPhone\t\tLoyalty Points\n");
                printf("─────────────────────────────────────────────────────────────────────────────\n");
                if (userRoot == NULL) {
                    printf("No users registered.\n");
                } else {
                    displayUsersInorder(userRoot);
                }
                pressEnter();
                break;
            }
            case 7: {
                clearScreen();
                printHeader("COMPLETE ORDER HISTORY");
                if (historyRoot == NULL) {
                    printf("No order history.\n");
                } else {
                    printf("Order ID\tCustomer\t\tStatus\t\t\tTotal\t\tOrder Time\n");
                    printf("─────────────────────────────────────────────────────────────────────────────────────────────\n");
                    displayOrderHistoryInorder(historyRoot);
                }
                pressEnter();
                break;
            }
            case 8: {
                clearScreen();
                printf("Enter Order ID to track: ");
                int orderId;
                scanf("%d", &orderId);
                displayOrderStatus(orderId, "admin", 1);
                pressEnter();
                break;
            }
            case 9: {
                clearScreen();
                char code[20];
                float discount;
                printf("Enter promo code: ");
                scanf("%s", code);
                printf("Enter discount percentage: ");
                scanf("%f", &discount);
                addPromoCode(code, discount);
                pressEnter();
                break;
            }
            case 10: {
                saveData();
                pressEnter();
                break;
            }
            case 11: {
                printf("Admin logging out...\n");
                break;
            }
            default: {
                printf("Invalid choice!\n");
                pressEnter();
            }
        }
    } while (choice != 11);
}

void userLogin() {
    char username[MAX_NAME];
    char password[MAX_PASS];
    
    printHeader("USER LOGIN");
    
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    User *user = searchUser(userRoot, username);
    if (user == NULL || strcmp(user->password, password) != 0) {
        printf("✗ Invalid username or password!\n");
        return;
    }
    
    printf("\n✓ Login successful!\n");
    userDashboard(username);
}

void userSignup() {
    char username[MAX_NAME];
    char password[MAX_PASS];
    char address[MAX_ADDR];
    char phone[MAX_PHONE];
    
    printHeader("USER SIGNUP");
    
    printf("Choose username: ");
    scanf("%s", username);
    
    /* Check if username exists */
    if (searchUser(userRoot, username) != NULL) {
        printf("✗ Username already exists!\n");
        return;
    }
    
    printf("Choose password: ");
    scanf("%s", password);
    printf("Enter address: ");
    scanf(" %[^\n]", address);
    printf("Enter phone number: ");
    scanf("%s", phone);
    
    User *newUser = createUser(username, password, address, phone);
    userRoot = insertUser(userRoot, newUser);
    
    printf("\n✓ Account created successfully! You can now login.\n");
}

void adminLogin() {
    char username[MAX_NAME];
    char password[MAX_PASS];
    
    printHeader("ADMIN LOGIN");
    
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    User *admin = searchUser(userRoot, username);
    if (admin != NULL && strcmp(admin->password, password) == 0) {
        printf("\n✓ Admin login successful!\n");
        adminDashboard();
    } else {
        printf("✗ Invalid admin credentials!\n");
    }
}

/* =============================== MAIN FUNCTION =============================== */
int main() {
    clearScreen();
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          ONLINE FOOD DELIVERY MANAGEMENT SYSTEM           ║\n");
    printf("║              with Order Tracking & Status                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    initializeSystem();
    
    int mainChoice;
    
    do {
        printf("\n");
        printHeader("MAIN MENU");
        printf("1. User Login\n");
        printf("2. User Signup\n");
        printf("3. Admin Login\n");
        printf("4. Browse Menu (Guest)\n");
        printf("5. Exit System\n");
        printLine();
        printf("Choice: ");
        scanf("%d", &mainChoice);
        
        switch(mainChoice) {
            case 1: userLogin(); break;
            case 2: userSignup(); break;
            case 3: adminLogin(); break;
            case 4: 
                clearScreen();
                displayAllMenu();
                pressEnter();
                break;
            case 5: 
                saveData();
                printf("\nThank you for using Online Food Delivery System!\n");
                break;
            default: 
                printf("Invalid choice! Please try again.\n");
        }
        
    } while (mainChoice != 5);
    
    return 0;
}
