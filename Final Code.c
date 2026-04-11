#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 18
#define MAX_COLS 3
#define MAX_TABLES (MAX_ROWS * MAX_COLS) // 54 Tables
#define DB_FILE "restaurant_db.txt"

// --- DATA STRUCTURES ---
struct TableBooking {
    int tableId; 
    char customerName[50];
    int row;
    int col;
    int isOccupied;
    float currentBill;
};

// --- FUNCTION PROTOTYPES ---
void loadData(struct TableBooking *tables);
void saveData(struct TableBooking *tables);
void displayTableMap(struct TableBooking *tables);
void bookAndOrder(struct TableBooking *tables);
void checkoutTable(struct TableBooking *tables);
void processOrder(struct TableBooking *currentTable); 
void clearInputBuffer();

// --- MAIN FUNCTION ---
int main() {
    struct TableBooking tables[MAX_TABLES];
    int choice;

    // Initialize Tables
    for (int i = 0; i < MAX_TABLES; i++) {
        tables[i].tableId = i + 1;
        tables[i].row = i / MAX_COLS;
        tables[i].col = i % MAX_COLS;
        tables[i].isOccupied = 0;
        tables[i].currentBill = 0.0;
        strcpy(tables[i].customerName, "");
    }

    // Load previous state from file
    loadData(tables);

    do {
        printf("\n==========================================\n");
        printf("            MIT Canteen SHREERAJ            \n");
        printf("==========================================\n");
        printf(" 1. View Table Map\n");
        printf(" 2. Book Table & Take Order\n");
        printf(" 3. Checkout / Clear Table\n");
        printf(" 4. Save & Exit\n");
        printf("==========================================\n");
        printf(" Enter Choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input!\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                displayTableMap(tables);
                break;
            case 2:
                bookAndOrder(tables);
                break;
            case 3:
                checkoutTable(tables);
                break;
            case 4:
                saveData(tables);
                printf("\n[INFO] Data saved. Shutting down...\n");
                break;
            default:
                printf("\n[ERROR] Invalid choice.\n");
        }
    } while (choice != 4);

    return 0;
}

// --- FILE HANDLING ---
void loadData(struct TableBooking *t) {
    FILE *fp = fopen(DB_FILE, "r");
    if (!fp) return; 

    int id, occupied;
    char name[50];
    float bill;

    // Format: ID,Name,Occupied,Bill
    while (fscanf(fp, "%d,%49[^,],%d,%f\n", &id, name, &occupied, &bill) == 4) {
        int idx = id - 1;
        if (idx >= 0 && idx < MAX_TABLES) {
            t[idx].tableId = id;
            strcpy(t[idx].customerName, name);
            t[idx].isOccupied = occupied;
            t[idx].currentBill = bill;
        }
    }
    fclose(fp);
    printf("[INIT] System loaded. Previous table states restored.\n");
}

void saveData(struct TableBooking *t) {
    FILE *fp = fopen(DB_FILE, "w");
    if (!fp) {
        printf("\n[ERROR] Could not save data!\n");
        return;
    }

    for (int i = 0; i < MAX_TABLES; i++) {
        // Only save occupied tables to save space/time
        if (t[i].isOccupied) {
            fprintf(fp, "%d,%s,%d,%.2f\n", 
                t[i].tableId, t[i].customerName, t[i].isOccupied, t[i].currentBill);
        }
    }
    fclose(fp);
}


void displayTableMap(struct TableBooking *t) {
    printf("\n      ====== TABLE MAP (1-54) ======\n");
    printf("      [Col 1]   [Col 2]   [Col 3]\n");
    
    for (int r = 0; r < MAX_ROWS; r++) {
        printf(" Row%02d ", r + 1);
        
        for (int c = 0; c < MAX_COLS; c++) {
            int tableNum = (r * MAX_COLS) + c + 1;
            int idx = tableNum - 1;

            if (t[idx].isOccupied) {
                printf(" [ X ]    "); 
            } else {
                printf(" [%02d ]    ", tableNum);
            }
        }
        printf("\n");
    }
    printf(" ------------------------------------\n");
    printf(" [ X ] = Occupied/Reserved\n");
}

void bookAndOrder(struct TableBooking *t) {
    int tableNum;
    displayTableMap(t);

    printf("\nEnter Table Number (1-54) to Book: ");
    if (scanf("%d", &tableNum) != 1) {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (tableNum < 1 || tableNum > 54) {
        printf("[ERROR] Table does not exist.\n");
        return;
    }

    int idx = tableNum - 1;

    if (t[idx].isOccupied) {
        printf("[ERROR] Table %d is already occupied by %s!\n", tableNum, t[idx].customerName);
        printf("Use 'Checkout' option to clear it first.\n");
        return;
    }

    // New Booking
    printf("Enter Customer Name: ");
    scanf(" %[^\n]", t[idx].customerName);
    
    t[idx].isOccupied = 1;
    t[idx].currentBill = 0; // Reset bill for new customer

    // Go to the main menu logic
    processOrder(&t[idx]);
    
    saveData(t);
}

void checkoutTable(struct TableBooking *t) {
    int tableNum;
    printf("\nEnter Table Number to Checkout/Clear: ");
    scanf("%d", &tableNum);

    if (tableNum < 1 || tableNum > 54) {
        printf("[ERROR] Invalid Table.\n");
        return;
    }

    int idx = tableNum - 1;

    if (!t[idx].isOccupied) {
        printf("[INFO] Table %d is already empty.\n", tableNum);
        return;
    }

    printf("\n---------------------------------");
    printf("\n CHECKOUT RECEIPT");
    printf("\n Table: %d | Customer: %s", t[idx].tableId, t[idx].customerName);
    printf("\n Total Bill Cleared: Rs. %.2f", t[idx].currentBill);
    printf("\n---------------------------------\n");

    // Reset Table
    t[idx].isOccupied = 0;
    strcpy(t[idx].customerName, "");
    t[idx].currentBill = 0;
    
    printf("[SUCCESS] Table %d is now free.\n", tableNum);
    saveData(t);
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- MENU & BILLING LOGIC (Merged) ---
void processOrder(struct TableBooking *currentTable) {
    int mainChoice, subChoice, qty;
    float currentTotal = 0;
    int repeat;
    float gst, discount, grandTotal;
    float itemPrice = 0;

    do {
        itemPrice = 0; // Reset per iteration
        printf("\n--------------- MAIN MENU ---------------");
        printf("\n1. Hot Morning");
        printf("\n2. Chat Dish");
        printf("\n3. Pav Family");
        printf("\n4. South Indian");
        printf("\n5. Starter");
        printf("\n6. Paratha");
        printf("\n7. Sabji");
        printf("\n8. Roti");
        printf("\n9. Rice");
        printf("\n10. Dessert");
        printf("\n11. Beverages");
        printf("\n12. Chinese");
        printf("\n13. Fast Food");
        printf("\n-----------------------------------------");
        printf("\nEnter your choice: ");
        scanf("%d", &mainChoice);

        switch (mainChoice) {
            case 1:
                printf("\n1. Tea (20)\n2. Special Tea (30)\n3. Coffee (40)\n4. Cream Roll (35)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=20; else if(subChoice==2) itemPrice=30;
                else if(subChoice==3) itemPrice=40; else if(subChoice==4) itemPrice=35;
                break;

            case 2:
                printf("\n1. Samosa Chat (50)\n2. Kachori Chat (60)\n3. Bhel (40)\n4. Aloo Chat (45)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=50; else if(subChoice==2) itemPrice=60;
                else if(subChoice==3) itemPrice=40; else if(subChoice==4) itemPrice=45;
                break;

            case 3:
                printf("\n1. Pav Bhaji (120)\n2. Wada Pav (30)\n3. Misal Pav (90)\n4. Masala Pav (80)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=120; else if(subChoice==2) itemPrice=30;
                else if(subChoice==3) itemPrice=90; else if(subChoice==4) itemPrice=80;
                break;

            case 4:
                printf("\n1. Paper Dosa (120)\n2. Masala Dosa (140)\n3. Mysore Dosa (150)");
                printf("\n4. Onion Uttapa (130)\n5. Tomato Uttapa (130)\n6. Idli Sambar (70)\n7. Sambar Wada (90)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=120; else if(subChoice==2) itemPrice=140;
                else if(subChoice==3) itemPrice=150; else if(subChoice==4||subChoice==5) itemPrice=130;
                else if(subChoice==6) itemPrice=70; else if(subChoice==7) itemPrice=90;
                break;

            case 5:
                printf("\n1. Poha (40)\n2. Upma (50)\n3. Paneer 65 (120)\n4. Veg 65 (100)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=40; else if(subChoice==2) itemPrice=50;
                else if(subChoice==3) itemPrice=120; else if(subChoice==4) itemPrice=100;
                break;

            case 6:
                printf("\n1. Aloo Paratha (70)\n2. Gobi Paratha (75)\n3. Palak Paratha (80)\n4. Cheese Paratha (100)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=70; else if(subChoice==2) itemPrice=75;
                else if(subChoice==3) itemPrice=80; else if(subChoice==4) itemPrice=100;
                break;

            case 7:
                printf("\nSabji Section (All @ 180)");
                printf("\n1. Chana Masala\n2. Cholle Bhature\n3. Bhendi Masala\n4. Paneer Masala");
                printf("\n5. Veg Handi\n6. Veg Kolhapuri\n7. Paneer Pasanda");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if (subChoice >= 1 && subChoice <= 7) itemPrice = 180;
                break;

            case 8:
                printf("\n1. Chapati (20)\n2. Butter Naan (40)\n3. Naan (35)");
                printf("\n4. Tandoori Roti (25)\n5. Bread (20)\n6. Bun Maska (30)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1||subChoice==5) itemPrice=20; else if(subChoice==2) itemPrice=40;
                else if(subChoice==3) itemPrice=35; else if(subChoice==4) itemPrice=25;
                else if(subChoice==6) itemPrice=30;
                break;

            case 9:
                printf("\n1. Rice (80)\n2. Jeera Rice (100)\n3. Schezwan Rice (130)");
                printf("\n4. Hong Kong Rice (150)\n5. Fried Rice (120)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=80; else if(subChoice==2) itemPrice=100;
                else if(subChoice==3) itemPrice=130; else if(subChoice==4) itemPrice=150;
                else if(subChoice==5) itemPrice=120;
                break;

            case 10:
                printf("\n1. Chocolate Pastry (70)\n2. Pineapple Pastry (65)\n3. Ice Cream (60)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=70; else if(subChoice==2) itemPrice=65;
                else if(subChoice==3) itemPrice=60;
                break;

            case 11:
                printf("\n1. Lemonade (40)\n2. Lassi (50)\n3. Nescafe Can (60)");
                printf("\n4. Cold Drink (45)\n5. Milkshake (80)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=40; else if(subChoice==2) itemPrice=50;
                else if(subChoice==3) itemPrice=60; else if(subChoice==4) itemPrice=45;
                else if(subChoice==5) itemPrice=80;
                break;

            case 12:
                printf("\n1. Veg Manchurian (150)\n2. Veg Noodles (140)");
                printf("\n3. Hakka Noodles (150)\n4. Schezwan Noodles (160)\n5. Paneer Chilly (180)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1||subChoice==3) itemPrice=150; else if(subChoice==2) itemPrice=140;
                else if(subChoice==4) itemPrice=160; else if(subChoice==5) itemPrice=180;
                break;

            case 13:
                printf("\n1. Pizza (200)\n2. Burger (120)\n3. French Fries (100)");
                printf("\n4. Veg Sandwich (90)\n5. Cheese Sandwich (110)");
                printf("\nEnter choice: "); scanf("%d", &subChoice);
                if(subChoice==1) itemPrice=200; else if(subChoice==2) itemPrice=120;
                else if(subChoice==3) itemPrice=100; else if(subChoice==4) itemPrice=90;
                else if(subChoice==5) itemPrice=110;
                break;

            default:
                printf("Invalid choice!");
                itemPrice = 0;
        }

        if (itemPrice > 0) {
            printf("Enter quantity: ");
            scanf("%d", &qty);
            currentTotal += (itemPrice * qty);
            printf("Added to bill. Running Total: Rs. %.2f\n", currentTotal);
        } else {
            if (mainChoice >=1 && mainChoice <=13) printf("Invalid Dish Selection.\n");
        }

        printf("\nOrder more? (1 = Yes, 0 = No): ");
        scanf("%d", &repeat);

    } while (repeat == 1);

    
    gst = currentTotal * 0.05;
    discount = (currentTotal >= 1000) ? currentTotal * 0.10 : 0;
    grandTotal = currentTotal + gst - discount;

    
    currentTable->currentBill = grandTotal;

    
    printf("\n=================================");
    printf("\n           ORDER SUMMARY         ");
    printf("\n=================================");
    printf("\nCustomer     : %s", currentTable->customerName);
    printf("\nTable Number : %d", currentTable->tableId);
    printf("\n---------------------------------");
    printf("\nSub Total    : Rs. %.2f", currentTotal);
    printf("\nGST (5%%)     : Rs. %.2f", gst);
    printf("\nDiscount     : Rs. %.2f", discount);
    printf("\n---------------------------------");
    printf("\nFinal Amount : Rs. %.2f", grandTotal);
    printf("\n=================================");
    printf("\n[INFO] Table Marked Occupied. \nSelect 'Checkout' in main menu to pay and clear later.\n");
}