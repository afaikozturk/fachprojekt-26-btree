#!/usr/bin/env python3

import pandas as pd
import seaborn as sns
import sys
import matplotlib.pyplot as plt
import re

if len(sys.argv) != 2:
    print("Usage: ./plot.py <input_file>")
    sys.exit(1)

input_file = sys.argv[1]
parsed_data = []

# 1. Daten aus der Log-Datei extrahieren
with open(input_file, 'r') as f:
    for line in f:
        # Überspringe Zeilen, die keine Benchmark-Daten enthalten (wie "Workload: ...")
        if "phase[" not in line:
            continue
            
        # Nutze Regex, um Phase, Iteration und die restlichen Metriken zu trennen
        # Sucht nach dem Muster: "Insert phase[0]: <Rest der Zeile>"
        match = re.match(r"(Insert|Lookup) phase\[(\d+)\]:\s*(.*)", line.strip())
        if match:
            phase = match.group(1)
            iteration = int(match.group(2))
            metrics_string = match.group(3)
            
            # Splitte den Rest am '|' Symbol
            metric_pairs = metrics_string.split('|')
            for pair in metric_pairs:
                if ':' in pair:
                    metric_name, metric_value = pair.split(':')
                    parsed_data.append({
                        'Phase': phase,
                        'Iteration': iteration,
                        'Metric': metric_name.strip(),
                        # float() verarbeitet C++ e-Notation (wie 6.46e+09) automatisch korrekt
                        'Value': float(metric_value.strip()) 
                    })

if not parsed_data:
    print(f"Fehler: Keine verwertbaren Daten in '{input_file}' gefunden.")
    sys.exit(1)

# 2. DataFrame für Seaborn erstellen
df = pd.DataFrame(parsed_data)

# 3. Plotting
# Wir nutzen catplot, um für jede Metrik ein eigenes Sub-Diagramm (Facet) zu bauen.
# sharey=False ist wichtig, damit Sekunden und Milliarden-Zyklen ihre eigene Skala bekommen.
g = sns.catplot(
    data=df,
    x='Iteration',
    y='Value',
    hue='Phase',
    col='Metric',
    kind='bar',
    col_wrap=2,        # 2 Diagramme pro Zeile
    sharey=False,      # Unabhängige Y-Achsen
    height=4,          # Höhe der Einzeldiagramme
    aspect=1.2         # Seitenverhältnis
)

# Layout und Titel anpassen
g.fig.suptitle('B-Tree Performance: Insert vs. Lookup', y=1.03, fontsize=14, fontweight='bold')
plt.tight_layout()

# 4. Speichern
output_file = 'performance_plot.pdf'
plt.savefig(output_file, bbox_inches='tight')
print(f"Plot erfolgreich erstellt und als '{output_file}' gespeichert.")
