<?php

namespace Drupal\hello\Access;


use Drupal\Core\Access\AccessCheckInterface;
use Drupal\Core\Access\AccessResult;
use Drupal\Core\Session\AccountInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\Routing\Route;

class HelloAccessCheck implements AccessCheckInterface {

    public function applies(Route $route)
    {
        // TODO: Implement applies() method.
        return NULL;
    }

    public function access(Route $route, Request $request = NULL, AccountInterface $account){
        $param= $route->getRequirement('_access_hello');
            $time=$account->getAccount()->created;
        $time_now=time();
        if (($time_now-($param*3600)) >= $time){

            return AccessResult::allowed();
        }
        return AccessResult::forbidden();
    }
}