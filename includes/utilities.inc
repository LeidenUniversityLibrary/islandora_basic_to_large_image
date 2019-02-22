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
  // Save old basic image
  if (isset($object['OBJ'])) {
    $oldlabel = $object['OBJ']->label;
    $oldmime = $object['OBJ']->mimetype;
    $oldbasicfilepath = file_create_filename('old_basic_image', 'temporary://');
    $object['OBJ']->getContent($oldbasicfilepath);
  }

  // Remove existing OBJ and derivitive datastreams if needed.
  $deletedsids = array('OBJ', 'TN', 'RELS-INT', 'TECHMD', 'MEDIUM_SIZE');
  foreach ($deletedsids as $dsid) {
    if (isset($object[$dsid])) {
      if (islandora_delete_datastream($object[$dsid]) !== TRUE) {
        return FALSE;
      }
    }
  }

  // Change the content model if needed.
  $newcmodel = islandora_basic_to_large_image_cmodel_large_image();
  if (count($object->relationships->get('info:fedora/fedora-system:def/model#', 'hasModel', $newcmodel)) === 0) {
    $object->relationships->autoCommit = FALSE;
    $object->relationships->remove('info:fedora/fedora-system:def/model#', 'hasModel');
    $object->relationships->add('info:fedora/fedora-system:def/model#', 'hasModel', $newcmodel);
    $object->relationships->commitRelationships();
  }

  // Ingest the new OBJ datastream.
  $ds = $object->constructDatastream('OBJ', 'M');
  $mime_detector = new MimeDetect();
  $ds->mimetype = $mime_detector->getMimetype($filepath);
  $ds->setContentFromFile($filepath, TRUE);
  $ds->label = basename($filepath);
  $object->ingestDatastream($ds);

  // Ingest old basic image.
  if (isset($oldbasicfilepath) && !isset($object['OLDBASIC'])) {
    $ds = $object->constructDatastream('OLDBASIC', 'M');
    $ds->mimetype = $oldmime;
    $ds->setContentFromFile($oldbasicfilepath, TRUE);
    file_unmanaged_delete($oldbasicfilepath);
    $ds->label = $oldlabel;
    $object->ingestDatastream($ds);
  }
  $object->refresh();

  return TRUE;
}

function islandora_basic_to_large_image_process_object_operation($object, $path, &$context) {
  module_load_include('inc', 'islandora_basic_to_large_image', 'includes/utilities');
  $result = islandora_basic_to_large_image_process_object($object, $path);
  if ($result) {
    $context['results'][] = $object->id;
  }
}

function islandora_basic_to_large_image_derivatives_operation($object, &$context) {
  return;
  module_load_include('inc', 'islandora', 'includes/derivatives');
  islandora_remove_defer_derivatives_flag($object);
  $results = islandora_do_derivatives($object, array('force' => TRUE));
}

function islandora_basic_to_large_image_process_object_batch_finished($success, $results, $operations) {
  if ($success && count($results) > 0) {
    drupal_set_message(t('Upgrade was succesful.'), 'status');
  }
  else {
    drupal_set_message(t('Upgrade failed.'), 'error');
  }
}
