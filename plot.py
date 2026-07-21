#!/usr/bin/env python3

import pandas as pd
import seaborn as sns
import sys
import matplotlib.pyplot as plt
import re

# Wir erwarten jetzt 2 Eingabedateien
if len(sys.argv) != 3:
    print("Usage: ./plot.py <baseline_log> <optimized_log>")
    sys.exit(1)

baseline_file = sys.argv[1]
optimized_file = sys.argv[2]
parsed_data = []

# Funktion zum Einlesen und Parsen einer Log-Datei
def parse_file(filepath, version_name):
    data = []
    with open(filepath, 'r') as f:
        for line in f:
            if "phase[" not in line:
                continue
            
            match = re.match(r"(Insert|Lookup) phase\[(\d+)\]:\s*(.*)", line.strip())
            if match:
                phase = match.group(1)
                iteration = int(match.group(2))
                metrics_string = match.group(3)
                
                metric_pairs = metrics_string.split('|')
                for pair in metric_pairs:
                    if ':' in pair:
                        metric_name, metric_value = pair.split(':')
                        data.append({
                            'Version': version_name, # NEU: Markierung ob Baseline oder Optimized
                            'Phase': phase,
                            'Iteration': iteration,
                            'Metric': metric_name.strip(),
                            'Value': float(metric_value.strip()) 
                        })
    return data

# Beide Dateien einlesen
parsed_data.extend(parse_file(baseline_file, 'Baseline (Old)'))
parsed_data.extend(parse_file(optimized_file, 'Optimized (New)'))

if not parsed_data:
    print("Fehler: Keine verwertbaren Daten in den Dateien gefunden.")
    sys.exit(1)

# DataFrame für Seaborn erstellen
df = pd.DataFrame(parsed_data)

# Plotting: Wir plotten 'Phase' auf der X-Achse und nutzen 'Version' für die Farbe (hue)
g = sns.catplot(
    data=df,
    x='Phase',           # X-Achse: Insert vs Lookup
    y='Value',
    hue='Version',       # Farbe: Baseline vs Optimized
    col='Metric',        # Ein Sub-Plot pro Metrik (seconds, instructions, etc.)
    kind='bar',
    errorbar='sd',       # Zeigt die Standardabweichung über die Iterationen als Fehlerbalken
    col_wrap=2,          # 2 Diagramme pro Zeile
    sharey=False,        # Unabhängige Y-Achsen
    height=4,            
    aspect=1.2           
)

# Layout und Titel anpassen
g.fig.suptitle('B-Tree Performance: Baseline vs. Optimized', y=1.03, fontsize=14, fontweight='bold')

# Y-Achsen etwas schöner formatieren (verhindert dass Labels abgeschnitten werden)
for ax in g.axes.flat:
    ax.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

plt.tight_layout()

# Speichern
output_file = 'performance_comparison_plot.pdf'
plt.savefig(output_file, bbox_inches='tight')
print(f"Plot erfolgreich erstellt und als '{output_file}' gespeichert.")