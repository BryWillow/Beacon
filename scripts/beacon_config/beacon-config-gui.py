import tkinter as tk
from tkinter import filedialog, messagebox
import json
import os

class ConfigEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Beacon Config Editor")
        self.geometry("600x400")
        self.config_path = None
        self.config_data = {}

        # File menu
        menubar = tk.Menu(self)
        filemenu = tk.Menu(menubar, tearoff=0)
        filemenu.add_command(label="Open", command=self.open_config)
        filemenu.add_command(label="Save", command=self.save_config)
        menubar.add_cascade(label="File", menu=filemenu)
        self.config(menu=menubar)

        # Simple fields for demo (expand as needed)
        self.fields = {}
        self.add_field("market_data_host", "Market Data Host")
        self.add_field("market_data_port", "Market Data Port")
        self.add_field("exchange_host", "Exchange Host")
        self.add_field("exchange_port", "Exchange Port")

        tk.Button(self, text="Save", command=self.save_config).pack(pady=10)

    def add_field(self, key, label):
        frame = tk.Frame(self)
        frame.pack(fill="x", padx=10, pady=5)
        tk.Label(frame, text=label, width=20, anchor="w").pack(side="left")
        entry = tk.Entry(frame)
        entry.pack(side="left", fill="x", expand=True)
        self.fields[key] = entry

    def open_config(self):
        path = filedialog.askopenfilename(filetypes=[("JSON files", "*.json")])
        if path:
            self.config_path = path
            with open(path, "r") as f:
                self.config_data = json.load(f)
            # Populate fields (demo keys)
            self.fields["market_data_host"].delete(0, tk.END)
            self.fields["market_data_host"].insert(0, self.config_data.get("market_data", {}).get("host", ""))
            self.fields["market_data_port"].delete(0, tk.END)
            self.fields["market_data_port"].insert(0, self.config_data.get("market_data", {}).get("port", ""))
            self.fields["exchange_host"].delete(0, tk.END)
            self.fields["exchange_host"].insert(0, self.config_data.get("exchange", {}).get("host", ""))
            self.fields["exchange_port"].delete(0, tk.END)
            self.fields["exchange_port"].insert(0, self.config_data.get("exchange", {}).get("port", ""))

    def save_config(self):
        if not self.config_path:
            self.config_path = filedialog.asksaveasfilename(defaultextension=".json")
        if self.config_path:
            # Update config_data from fields
            self.config_data["market_data"] = {
                "host": self.fields["market_data_host"].get(),
                "port": int(self.fields["market_data_port"].get() or 0)
            }
            self.config_data["exchange"] = {
                "host": self.fields["exchange_host"].get(),
                "port": int(self.fields["exchange_port"].get() or 0)
            }
            with open(self.config_path, "w") as f:
                json.dump(self.config_data, f, indent=2)
            messagebox.showinfo("Saved", f"Configuration saved to {self.config_path}")

if __name__ == "__main__":
    app = ConfigEditor()
    app.mainloop()
