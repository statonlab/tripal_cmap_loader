The only goal of this module is to provide a Tripal 3 importer for Cmap files that conforms to the [Chado map module](http://gmod.org/wiki/Chado_Map_Module) schematic.  We hope to use existing modules for display (ie [cmapjs by legume federation](https://github.com/LegumeFederation/cmap-js)).


## Expected CMAP data
The below table shows an example CMAP file.  The importer will load features in assuming that the *accession* is the *unique name* and the *name* is the *feature name*.
currently we ignore the is_landmark and feature_aliases columns.


| map_acc       | map_name | map_start | map_stop | feature_name | feature_accession | feature_aliases | feature_start | feature_stop | feature_type_acc | is_landmark |
|---------------|----------|-----------|----------|--------------|-------------------|-----------------|---------------|--------------|------------------|-------------|
| C_mollisima_A | A        | 0         | 90.4     | CmSI0385     | CmSI0385          |                 | 0             | 0            | SSR              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP01340   | CmSNP01340        |                 | 1.1           | 1.1          | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP01086   | CmSNP01086        |                 | 3.5           | 3.5          | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP00635   | CmSNP00635        |                 | 6.9           | 6.9          | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSI0407     | CmSI0407          |                 | 7.5           | 7.5          | SSR              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP00135   | CmSNP00135        |                 | 8.5           | 8.5          | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSI0525     | CmSI0525          |                 | 9.1           | 9.1          | SSR              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP01101   | CmSNP01101        |                 | 13.4          | 13.4         | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP00862   | CmSNP00862        |                 | 14.8          | 14.8         | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP00251   | CmSNP00251        |                 | 17.1          | 17.1         | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP01050   | CmSNP01050        |                 | 19.5          | 19.5         | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4     | CmSNP00977   | CmSNP00977        |                 | 20.8          | 20.8         | SNP              | 0           |
