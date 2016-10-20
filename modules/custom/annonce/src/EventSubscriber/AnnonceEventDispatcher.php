<?php

namespace Drupal\annonce\EventSubscriber;

use Symfony\Component\EventDispatcher\EventSubscriberInterface;
use Symfony\Component\EventDispatcher\Event;
use Drupal\Core\Session\AccountProxy;
use Symfony\Component\HttpKernel\Event\KernelEvent;
use Symfony\Component\HttpKernel\KernelEvents;

use Drupal\Core\Routing\CurrentRouteMatch;
use \Drupal\Core\Database\Connection;

/**
 * Class AnnonceEventDispatcher.
 *
 * @package Drupal\annonce
 */
class AnnonceEventDispatcher implements EventSubscriberInterface {

  /**
   * Drupal\Core\Session\AccountProxy definition.
   *
   * @var Drupal\Core\Session\AccountProxy
   */
  protected $current_user;
  protected $current_route_match;
  protected $database;

  /**
   * Constructor.
   */
  public function __construct(AccountProxy $current_user, CurrentRouteMatch $current_route_match, Connection $database) {
    $this->current_user = $current_user;
    $this->current_route_match = $current_route_match;
    $this->database = $database;
  }

  public function DisplayUserName()
  {
    if ($this->current_route_match->getRouteName() == 'entity.annonce.canonical') {
          Drupal_set_message('PLOP ' . $this->current_user->getAccountName() . '! Current route : ' .
              $this->current_route_match->getRouteName(), 'status');

          $database=$this->database;
          $id_annonce = $this->current_route_match->getRawParameter('annonce');
          $update_time = time();
          $user_id = $this->current_user->id();

          $database->insert('annonce_history')
              ->fields(array('aid' => $id_annonce, 'update_time' => $update_time, 'uid' => $user_id))
              ->execute();
    }
  }

  /**
   * {@inheritdoc}
   */
  static function getSubscribedEvents() {
      $events[KernelEvents::REQUEST][]=array('DisplayUserName');
    return $events;
  }


}
