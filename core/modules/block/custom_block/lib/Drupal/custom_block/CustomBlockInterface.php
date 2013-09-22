<?php

/**
 * @file
 * Contains \Drupal\custom_block\Entity\CustomBlockInterface.
 */

namespace Drupal\custom_block;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityChangedInterface;

/**
 * Provides an interface defining a custom block entity.
 */
interface CustomBlockInterface extends ContentEntityInterface, EntityChangedInterface {

  /**
   * Sets the theme value.
   *
   * When creating a new custom block from the block library, the user is
   * redirected to the configure form for that block in the given theme. The
   * theme is stored against the block when the custom block add form is shown.
   *
   * @param string $theme
   *   The theme name.
   */
  public function setTheme($theme);

  /**
   * Gets the theme value.
   *
   * When creating a new custom block from the block library, the user is
   * redirected to the configure form for that block in the given theme. The
   * theme is stored against the block when the custom block add form is shown.
   *
   * @return string
   *   The theme name.
   */
  public function getTheme();

  /**
   * Gets the configured instances of this custom block.
   *
   * @return array
   *   Array of Drupal\block\Core\Plugin\Entity\Block entities.
   */
  public function getInstances();

  /**
   * Return the custom block type (bundle) name.
   *
   * @return string
   *   The name of the custom block type (bundle).
   */
  public function getCustomBlockType();

  /**
   * Returns the block description.
   *
   * @return string
   *   The description of this block.
   */
  public function getDescription();

  /**
   * Sets the description of the block to the given value.
   *
   * @param string $description
   *   The desired description.
   *
   * @return \Drupal\custom_block\CustomBlockInterface
   *   The class instance this method is called on.
   */
  public function setDescription($description);

  /**
   * Returns the block revision log message.
   *
   * @return string
   *   The log message for the current revision of this block.
   */
  public function getLogMessage();

  /**
   * Sets the the block revision log message to the given value.
   *
   * @param string
   *   The desired log message for the current revision of this block.
   *
   * @return \Drupal\custom_block\CustomBlockInterface
   *   The class instance this method is called on.
   */
  public function setLogMessage($log);
}
