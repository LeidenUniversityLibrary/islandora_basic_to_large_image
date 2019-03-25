<?php

/**
 * @file
 * Form for uploading large image to replace basic image.
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

/**
 * The form for handling the upload of a new large image.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_to_large_image_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora_large_image', 'includes/image_upload.form');
  $uploadimage = islandora_large_image_image_upload_form($form, $form_state);
  $result = array(
    'id' => array(
      '#type' => 'hidden',
      '#value' => $object->id,
    ),
    'upgrade_to_large' => array(
      '#type' => 'fieldset',
      '#title' => t('Upgrade to large image'),
      '#description' => t('Upload a new large image to upgrade this basic image to a large image.'),
      'file' => $uploadimage['file'],
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Upgrade this image'),
    ),
  );
  return $result;
}

function islandora_basic_to_large_image_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_basic_to_large_image', 'includes/utilities');
  $object = islandora_object_load($form_state['values']['id']);
  if ($form_state['values']['file']) {
    $file = file_load($form_state['values']['file']);
    $path = drupal_realpath($file->uri);
    $batch = islandora_basic_to_large_image_batch($object, $path);
    batch_set($batch);
  }
  $form_state['redirect'] = "islandora/object/{$form_state['values']['id']}/manage";
}
