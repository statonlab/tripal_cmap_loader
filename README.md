[![Build Status](https://travis-ci.org/statonlab/tripal_cmap_loader.svg?branch=master)](https://travis-ci.org/statonlab/tripal_cmap_loader)


## Tripal Cmap

This module currently provides a Tripal 3 importer for Cmap files that conforms to the [Chado map module](http://gmod.org/wiki/Chado_Map_Module) schematic.  We hope to use existing modules for display (ie [cmapjs by legume federation](https://github.com/LegumeFederation/cmap-js)).  A field utilizing cmap.js will be included in this module soon.


## Installation and setup

This module is not installable with drush.  Navigate to where your custom modules are installed and clone this repository.  For example:

``` bash
cd /var/www/html/sites/all/modules/custom/
git clone https://github.com/statonlab/tripal_cmap_loader.git
drush pm-enable tripal_cmap_loader -y
```

## Expected CMAP data


The below table shows an example CMAP file.  You can find full file in the example in  [the example folder](example/).  This example map is a [genetic map published here](https://link.springer.com/article/10.1007%2Fs11295-012-0576-6).


The importer will load features in assuming that the *accession* is the *unique name* and the *name* is the *feature name*.
currently we ignore the is_landmark and feature_aliases columns.



| map_acc       | map_name | map_start | map_stop | locus_acc | feature_name | feature_accession | feature_aliases | feature_start | feature_stop | feature_type_acc | is_landmark |
|---------------|----------|-----------|----------|--------------|-------------------|-----------------|---------------|--------------|------------------|-------------|-----|
| C_mollisima_A | A        | 0         | 90.4   | CmSI0385     | CmSI0385     | CmSI0385          |                 | 0             | 0            | microsatellite              | 0           |
| C_mollisima_A | A        | 0         | 90.4  | CmSNP01340     | CmSNP01340   | CmSNP01340        |                 | 1.1           | 1.1          | SNP              | 0           |
| C_mollisima_A | A        | 0         | 90.4   | CmSNP01086   | CmSNP01086   | CmSNP01086        |                 | 3.5           | 3.5          | SNP              | 0           |


## Using the importer
Before loading the file, create a featuremap (Tripal 3 bundle: Map) entity record.
  
  You will need to select the following when loading a cmap file:
  * organism
  * featuremap
  * sequence ontology term for the mapping features.
  
You must choose a single organism when loading the map: this is the organism that will be used for **new features created**.  Two types of things will be loaded into Chado: the mapping feature (a chromosome, a scaffold), and the marker feature (a SNP).  The type_id for the marker is taken from the `feature_type_acc` column: that term must be in the sequence ontology.  The mapping feature is chosen in the loader. 


# Cmap column to Chado mapping
Note that the cmap format is not consistent, or at least we have not found a definitive list of columns.  The loader expects the following 11 columns.  If your file is not 12 columns wide, it will not load.  The first column **must** be map_acc


| cmap column            | chado entry                                            |
|------------------------|--------------------------------------------------------|
| map_acc                | uniquename for mapping feature IE chromosome, scaffold |
| map_name               | name for mapping feature IE chromsome, scaffold        |
| map_start              | not used                                               |
| map_stop               | not used                                               |
| locus_accession        | uniquename for the locus feature                       |
| feature_name           | name for marker feature                                |
| feature_accession      | uniquename for marker feature                          |
| feature_aliases        | not used                                               |
| feature_start          | featurepos record for start                            |
| feature_stop           | featurepos record for stop                             |
| feature_type_accession | type_id for marker feature                             |
| is_landmark            | not used                                               |


## FPC

We have written a script to convert FPC format to cmap.  See [here for code](https://github.com/statonlab/fpc_to_cmap_converter)). 

## Using this module with tripal_map

If you would look to use the [Tripal Map](https://gitlab.com/mainlabwsu/tripal_map) module to visualize data loaded with this module, you need to make two small adjustments:


1. Replace the `tripal_map_genetic_markers_mview` mview.  

```sql

SELECT F.uniquename as marker_locus_name, F.feature_id as marker_locus_id, F2.uniquename as genetic_marker_name,
  C1.name as map_unit_type, C2.name as marker_type, FM.name as map_name, FM.featuremap_id as map_id, FMP.value as map_type,
  F3.name as linkage_group, F3.feature_id as linkage_group_id, FP.mappos as marker_pos, FPP.value as marker_pos_type,
  O.organism_id as organism_id, O.genus as genus, O.species as species, O.common_name as common_name
  FROM {feature} F
  INNER JOIN {feature_relationship} FR 	ON FR.subject_id = F.feature_id AND
    F.type_id = (SELECT cvterm_id  FROM {cvterm} WHERE name = 'biological_region' AND
    cv_id = (SELECT cv_id FROM {cv} WHERE name = 'sequence'))
     AND
    FR.type_id = (SELECT cvterm_id  FROM {cvterm} WHERE {cvterm}.name = 'instance_of' AND
    cv_id = (SELECT cv_id FROM {cv} WHERE name = 'OBO_REL'))

  INNER JOIN {feature} F2               	ON FR.object_id = F2.feature_id 
     AND
    FR.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'instance_of' AND
    cv_id = (SELECT cv_id FROM {cv} WHERE name = 'OBO_REL'))
    
  INNER JOIN {featurepos} FP            	ON F2.feature_id = FP.feature_id
  
  INNER JOIN {featuremap} FM    		ON FM.featuremap_id = FP.featuremap_id
  INNER JOIN {cvterm} C1                ON C1.cvterm_id = FM.unittype_id
  	INNER JOIN {cvterm} C2 ON C2.cvterm_id = F2.type_id
  INNER JOIN {featuremapprop} FMP       ON FMP.featuremap_id = FP.featuremap_id AND
   FMP.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'featuremap_type' AND
   cv_id = (SELECT cv_id FROM {cv} WHERE name = 'local'))
  INNER JOIN {featuremap_organism} FMO 	ON FMO.featuremap_id = FM.featuremap_id
  INNER JOIN {feature} F3 				ON FP.map_feature_id = F3.feature_id
  INNER JOIN {featureposprop} FPP 		ON FPP.featurepos_id = FP.featurepos_id
  INNER JOIN {cvterm} C 				ON C.cvterm_id = FPP.type_id
  INNER JOIN {organism} O 				ON FMO.organism_id = O.organism_id
  
  ```
  
  
  2.  Replace the `tripal_map_qtl_and_mtl_mview` mview
  
  
  ```sql
SELECT FM.name as map_name,
 FM.featuremap_id as map_id,
   FMP.value as map_type,
    F3.name as linkage_group,
 F3.feature_id as linkage_group_id,
 F.uniquename as marker_locus_name,
 C1.name as map_unit_type,
  C2.name as marker_type,
 FPP.value as marker_pos_type,
  FP.mappos as marker_pos,
O.organism_id as organism_id, O.genus as genus, O.species as species, O.common_name as common_name,
F.feature_id as feature_id, FPP.featurepos_id as featurepos_id
 FROM {feature} F
--F is the biological_region or parent marker locus
 INNER JOIN {feature_relationship} FR 	ON FR.subject_id = F.feature_id AND
   F.type_id = (SELECT cvterm_id  FROM {cvterm} WHERE name = 'biological_region' AND
   cv_id = (SELECT cv_id FROM cv WHERE name = 'sequence'))
    AND
   FR.type_id = (SELECT cvterm_id  FROM {cvterm} WHERE cvterm.name = 'instance_of' AND
   cv_id = (SELECT cv_id FROM cv WHERE name = 'OBO_REL'))
  
-- F2 is the marker feature itself
 INNER JOIN {feature} F2               	ON FR.object_id = F2.feature_id
-- This mview is just for QTLs
INNER JOIN {cvterm} C ON F2.type_id = C.cvterm_id AND (C.name = 'QTL' OR C.name = 'heritable_phenotypic_marker')
 INNER JOIN {featurepos} FP            	ON F2.feature_id = FP.feature_id
 INNER JOIN {featuremap} FM    		ON FM.featuremap_id = FP.featuremap_id
 INNER JOIN {cvterm} C1                ON C1.cvterm_id = FM.unittype_id
 -- C2 is the marker type term
   INNER JOIN {cvterm} C2 ON C2.cvterm_id = F2.type_id
 INNER JOIN {featuremapprop} FMP       ON FMP.featuremap_id = FP.featuremap_id AND
  FMP.type_id = (SELECT cvterm_id FROM cvterm WHERE name = 'featuremap_type' AND
  cv_id = (SELECT cv_id FROM cv WHERE name = 'local'))
 INNER JOIN {featuremap_organism} FMO 	ON FMO.featuremap_id = FM.featuremap_id
 --F3 is the parent feature of the map ie the linkage group
 INNER JOIN {feature} F3 				ON FP.map_feature_id = F3.feature_id
 INNER JOIN {featureposprop} FPP 		ON FPP.featurepos_id = FP.featurepos_id
 INNER JOIN {organism} O 				ON FMO.organism_id = O.organism_id


```
  
  3.  Ensure that the featuremap bundle ( IE Genetic Map) has a chado property with the `local:featuremap_type` term.  I reccommend setting a default value for this property so you won't forget.  **Note** you should only need to set this property yourself if the Tripal Entity you are using was custom created and has no type.
  
  
If your featuremap has this property set, and you've populated the altered **tripal_map_genetic_markers_mview** materialized view (`Data Storage -> Chado -> Materialized Views`, press "populate"), your field should show up on the organism and featuremap!  You might need to clear the cache (`drush cc all`) before the field appears on the organism.
  
  
