<?php

/**
 * @file
 * Contains \Drupal\Core\Menu\MenuLocalTaskManager.
 */

namespace Drupal\Core\Menu;

use Drupal\Component\Plugin\Exception\PluginException;
use Drupal\Core\Access\AccessManager;
use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\Core\Controller\ControllerResolverInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Language\LanguageManager;
use Drupal\Core\Plugin\DefaultPluginManager;
use Drupal\Core\Plugin\Discovery\ContainerDerivativeDiscoveryDecorator;
use Drupal\Core\Plugin\Discovery\YamlDiscovery;
use Drupal\Core\Plugin\Factory\ContainerFactory;
use Drupal\Core\Routing\RouteProviderInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Manages discovery and instantiation of menu local task plugins.
 *
 * This manager finds plugins that are rendered as local tasks (usually tabs).
 * Derivatives are supported for modules that wish to generate multiple tabs on
 * behalf of something else.
 */
class LocalTaskManager extends DefaultPluginManager {

  /**
   * {@inheritdoc}
   */
  protected $defaults = array(
    // (required) The name of the route this task links to.
    'route_name' => '',
    // Parameters for route variables when generating a link.
    'route_parameters' => array(),
    // The static title for the local task.
    'title' => '',
    // The plugin ID of the root tab.
    'tab_root_id' => '',
    // The plugin ID of the parent tab (or NULL for the top-level tab).
    'tab_parent_id' => NULL,
    // The weight of the tab.
    'weight' => NULL,
    // The default link options.
    'options' => array(),
    // Default class for local task implementations.
    'class' => 'Drupal\Core\Menu\LocalTaskDefault',
    // The plugin id. Set by the plugin system based on the top-level YAML key.
    'id' => '',
  );

  /**
   * A controller resolver object.
   *
   * @var \Drupal\Core\Controller\ControllerResolverInterface
   */
  protected $controllerResolver;

  /**
   * A request object.
   *
   * @var \Symfony\Component\HttpFoundation\Request
   */
  protected $request;

  /**
   * The plugin instances.
   *
   * @var array
   */
  protected $instances = array();

  /**
   * The route provider to load routes by name.
   *
   * @var \Drupal\Core\Routing\RouteProviderInterface
   */
  protected $routeProvider;

  /**
   * The access manager.
   *
   * @var \Drupal\Core\Access\AccessManager
   */
  protected $accessManager;

  /**
   * Constructs a \Drupal\Core\Menu\LocalTaskManager object.
   *
   * @param \Drupal\Core\Controller\ControllerResolverInterface $controller_resolver
   *   An object to use in introspecting route methods.
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The request object to use for building titles and paths for plugin instances.
   * @param \Drupal\Core\Routing\RouteProviderInterface $route_provider
   *   The route provider to load routes by name.
   * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
   *   The module handler.
   * @param \Drupal\Core\Cache\CacheBackendInterface $cache
   *   The cache backend.
   * @param \Drupal\Core\Language\LanguageManager $language_manager
   *   The language manager.
   * @param \Drupal\Core\Access\AccessManager $access_manager
   *   The access manager.
   */
  public function __construct(ControllerResolverInterface $controller_resolver, Request $request, RouteProviderInterface $route_provider, ModuleHandlerInterface $module_handler, CacheBackendInterface $cache, LanguageManager $language_manager, AccessManager $access_manager) {
    $this->discovery = new YamlDiscovery('local_tasks', $module_handler->getModuleDirectories());
    $this->discovery = new ContainerDerivativeDiscoveryDecorator($this->discovery);
    $this->factory = new ContainerFactory($this);
    $this->controllerResolver = $controller_resolver;
    $this->request = $request;
    $this->routeProvider = $route_provider;
    $this->accessManager = $access_manager;
    $this->alterInfo($module_handler, 'local_tasks');
    $this->setCacheBackend($cache, $language_manager, 'local_task', array('local_task' => TRUE));
  }

  /**
   * {@inheritdoc}
   */
  public function processDefinition(&$definition, $plugin_id) {
    parent::processDefinition($definition, $plugin_id);
     // If there is no route name, this is a broken definition.
    if (empty($definition['route_name'])) {
      throw new PluginException(sprintf('Plugin (%s) definition must include "route_name"', $plugin_id));
    }
  }

  /**
   * Gets the title for a local task.
   *
   * @param \Drupal\Core\Menu\LocalTaskInterface $local_task
   *   A local task plugin instance to get the title for.
   *
   * @return string
   *   The localized title.
   */
  public function getTitle(LocalTaskInterface $local_task) {
    $controller = array($local_task, 'getTitle');
    $arguments = $this->controllerResolver->getArguments($this->request, $controller);
    return call_user_func_array($controller, $arguments);
  }

  /**
   * Find all local tasks that appear on a named route.
   *
   * @param string $route_name
   *   The route for which to find local tasks.
   *
   * @return array
   *   Returns an array of task levels. Each task level contains instances
   *   of local tasks (LocalTaskInterface) which appear on the tab route.
   *   The array keys are the depths and the values are arrays of plugin
   *   instances.
   */
  public function getLocalTasksForRoute($route_name) {
    if (!isset($this->instances[$route_name])) {
      $this->instances[$route_name] = array();
      if ($cache = $this->cacheBackend->get($this->cacheKey . ':' . $route_name)) {
        $tab_root_ids = $cache->data['tab_root_ids'];
        $parents = $cache->data['parents'];
        $children = $cache->data['children'];
      }
      else {
        $definitions = $this->getDefinitions();
        // We build the hierarchy by finding all tabs that should
        // appear on the current route.
        $tab_root_ids = array();
        $parents = array();
        $children = array();
        foreach ($definitions as $plugin_id => $task_info) {
          if ($route_name == $task_info['route_name']) {
            $tab_root_ids[$task_info['tab_root_id']] = $task_info['tab_root_id'];
            // Tabs that link to the current route are viable parents
            // and their parent and children should be visible also.
            // @todo - this only works for 2 levels of tabs.
            // instead need to iterate up.
            $parents[$plugin_id] = TRUE;
            if (!empty($task_info['tab_parent_id'])) {
              $parents[$task_info['tab_parent_id']] = TRUE;
            }
          }
        }
        if ($tab_root_ids) {
          // Find all the plugins with the same root and that are at the top
          // level or that have a visible parent.
          foreach ($definitions  as $plugin_id => $task_info) {
            if (!empty($tab_root_ids[$task_info['tab_root_id']]) && (empty($task_info['tab_parent_id']) || !empty($parents[$task_info['tab_parent_id']]))) {
              // Concat '> ' with root ID for the parent of top-level tabs.
              $parent = empty($task_info['tab_parent_id']) ? '> ' . $task_info['tab_root_id'] : $task_info['tab_parent_id'];
              $children[$parent][$plugin_id] = $task_info;
            }
          }
        }
        $data = array(
          'tab_root_ids' => $tab_root_ids,
          'parents' => $parents,
          'children' => $children,
        );
        $this->cacheBackend->set($this->cacheKey . ':' . $route_name, $data, CacheBackendInterface::CACHE_PERMANENT, array('local_task'));
      }
      // Create a plugin instance for each element of the hierarchy.
      foreach ($tab_root_ids as $root_id) {
        // Convert the tree keyed by plugin IDs into a simple one with
        // integer depth.  Create instances for each plugin along the way.
        $level = 0;
        // We used this above as the top-level parent array key.
        $next_parent = '> ' . $root_id;
        do {
          $parent = $next_parent;
          $next_parent = FALSE;
          foreach ($children[$parent] as $plugin_id => $task_info) {
            $plugin = $this->createInstance($plugin_id);
            $this->instances[$route_name][$level][$plugin_id] = $plugin;
            // Normally, l() compares the href of every link with the current
            // path and sets the active class accordingly. But the parents of
            // the current local task may be on a different route in which
            // case we have to set the class manually by flagging it active.
            if (!empty($parents[$plugin_id]) && $route_name != $task_info['route_name']) {
              $plugin->setActive();
            }
            if (isset($children[$plugin_id])) {
              // This tab has visible children
              $next_parent = $plugin_id;
            }
          }
          $level++;
        } while ($next_parent);
      }

    }
    return $this->instances[$route_name];
  }

  /**
   * Gets the render array for all local tasks.
   *
   * @param string $current_route_name
   *   The route for which to make renderable local tasks.
   *
   * @return array
   *   A render array as expected by theme_menu_local_tasks.
   */
  public function getTasksBuild($current_route_name) {
    $tree = $this->getLocalTasksForRoute($current_route_name);
    $build = array();

    // Collect all route names.
    $route_names = array();
    foreach ($tree as $instances) {
      foreach ($instances as $child) {
        $route_names[] = $child->getRouteName();
      }
    }
    // Pre-fetch all routes involved in the tree. This reduces the number
    // of SQL queries that would otherwise be triggered by the access manager.
    $routes = $route_names ? $this->routeProvider->getRoutesByNames($route_names) : array();

    foreach ($tree as $level => $instances) {
      foreach ($instances as $plugin_id => $child) {
        // In order to get the Drupal path the base URL has to be stripped off.
        $route_name = $child->getRouteName();
        $route_parameters = $child->getRouteParameters($this->request);

        // Find out whether the user has access to the task.
        $access = $this->accessManager->checkNamedRoute($route_name, $route_parameters);
        if ($access) {
          // Need to flag the list element as active for a tab for the current
          // route or if the plugin is set active (i.e. the parent tab).
          $active = (($current_route_name == $route_name && (array_intersect_assoc($route_parameters, $this->request->attributes->all()) == $route_parameters)) || $child->getActive());
          // @todo It might make sense to use menu link entities instead of
          //   arrays.

          $link = array(
            'title' => $this->getTitle($child),
            'route_name' => $route_name,
            'route_parameters' => $route_parameters,
            'localized_options' => $child->getOptions($this->request),
          );
          $build[$level][$plugin_id] = array(
            '#theme' => 'menu_local_task',
            '#link' => $link,
            '#active' => $active,
            '#weight' => $child->getWeight(),
            '#access' => $access,
          );
        }
      }
    }
    return $build;
  }

}
