<?php

/**
 * @file
 * Utilities file for upgrading from basic to large image.
 *
 *
 *  Copyright 2019 Leiden University Library
 *
 *  This file is part of islandora_basic_to_large_image.
 *
 *  islandora_basic_to_large_image is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

function islandora_basic_to_large_image_process_object($object, $filepath) {
  // Check access.
  if (islandora_basic_to_large_image_access($object) !== TRUE) {
    return FALSE;
  }
  // Change the content model if needed.
  $newcmodel = islandora_basic_to_large_image_cmodel_large_image();
  if (count($object->relationships->get('info:fedora/fedora-system:def/model#', 'hasModel', $newcmodel)) === 0) {  
    $object->relationships->autoCommit = FALSE;
    $object->relationships->remove('info:fedora/fedora-system:def/model#', 'hasModel');  
    $object->relationships->add('info:fedora/fedora-system:def/model#', 'hasModel', $newcmodel);  
    $object->relationships->commitRelationships();
  }

  // Remove existing derivitive datastreams if needed.
  $deletedsids = array('TN', 'RELS-INT', 'TECHMD', 'MEDIUM_SIZE');
  foreach ($deletedsids as $dsid) {
    if (isset($object[$dsid])) {
      if (islandora_delete_datastream($object[$dsid]) !== TRUE) {
        return FALSE;
      }
    }
  }

  // Ingest the new OBJ datastream.
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }
  $ds->setContentFromFile($filepath, FALSE);

  return TRUE;
}
