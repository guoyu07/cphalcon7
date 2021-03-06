
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#include "di/service.h"
#include "di/serviceinterface.h"
#include "di/service/builder.h"
#include "di/exception.h"
#include "events/managerinterface.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/operators.h"

#include "interned-strings.h"
#include "internal/arginfo.h"

/**
 * Phalcon\DI\Service
 *
 * Represents individually a service in the services container
 *
 *<code>
 * $service = new Phalcon\DI\Service('request', 'Phalcon\Http\Request');
 * $request = $service->resolve();
 *<code>
 *
 */
zend_class_entry *phalcon_di_service_ce;

PHP_METHOD(Phalcon_DI_Service, __construct);
PHP_METHOD(Phalcon_DI_Service, getName);
PHP_METHOD(Phalcon_DI_Service, setShared);
PHP_METHOD(Phalcon_DI_Service, isShared);
PHP_METHOD(Phalcon_DI_Service, setSharedInstance);
PHP_METHOD(Phalcon_DI_Service, setDefinition);
PHP_METHOD(Phalcon_DI_Service, getDefinition);
PHP_METHOD(Phalcon_DI_Service, resolve);
PHP_METHOD(Phalcon_DI_Service, setParameter);
PHP_METHOD(Phalcon_DI_Service, getParameter);
PHP_METHOD(Phalcon_DI_Service, isResolved);
PHP_METHOD(Phalcon_DI_Service, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di_service___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, definition)
	ZEND_ARG_INFO(0, shared)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_di_service_method_entry[] = {
	PHP_ME(Phalcon_DI_Service, __construct, arginfo_phalcon_di_service___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_DI_Service, getName, arginfo_phalcon_di_serviceinterface_getname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, setShared, arginfo_phalcon_di_serviceinterface_setshared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, isShared, arginfo_phalcon_di_serviceinterface_isshared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, setSharedInstance, arginfo_phalcon_di_service_setsharedinstance, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, setDefinition, arginfo_phalcon_di_serviceinterface_setdefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, getDefinition, arginfo_phalcon_di_serviceinterface_getdefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, resolve, arginfo_phalcon_di_serviceinterface_resolve, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, setParameter, arginfo_phalcon_di_service_setparameter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, getParameter, arginfo_phalcon_di_service_getparameter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, isResolved, arginfo_phalcon_di_serviceinterface_isresolved, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Service, __set_state, arginfo___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\DI\Service initializer
 */
PHALCON_INIT_CLASS(Phalcon_DI_Service){

	PHALCON_REGISTER_CLASS(Phalcon\\DI, Service, di_service, phalcon_di_service_method_entry, 0);

	zend_declare_property_null(phalcon_di_service_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_service_ce, SL("_definition"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_di_service_ce, SL("_shared"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_di_service_ce, SL("_resolved"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_service_ce, SL("_sharedInstance"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_di_service_ce, 1, phalcon_di_serviceinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\DI\Service
 *
 * @param string $name
 * @param mixed $definition
 * @param boolean $shared
 */
PHP_METHOD(Phalcon_DI_Service, __construct){

	zval *name, *definition, *shared = NULL;

	phalcon_fetch_params(0, 2, 1, &name, &definition, &shared);
	PHALCON_ENSURE_IS_STRING(name);

	phalcon_update_property_zval(getThis(), SL("_name"), name);
	phalcon_update_property_zval(getThis(), SL("_definition"), definition);
	if (shared) {
		PHALCON_ENSURE_IS_BOOL(shared);
		phalcon_update_property_zval(getThis(), SL("_shared"), shared);
	}
}

/**
 * Returns the service's name
 *
 * @param string
 */
PHP_METHOD(Phalcon_DI_Service, getName){

	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Sets if the service is shared or not
 *
 * @param boolean $shared
 */
PHP_METHOD(Phalcon_DI_Service, setShared){

	zval *shared;
	phalcon_fetch_params(0, 1, 0, &shared);
	phalcon_update_property_zval(getThis(), SL("_shared"), shared);
}

/**
 * Check whether the service is shared or not
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_DI_Service, isShared){

	RETURN_MEMBER(getThis(), "_shared");
}

/**
 * Sets/Resets the shared instance related to the service
 *
 * @param mixed $sharedInstance
 */
PHP_METHOD(Phalcon_DI_Service, setSharedInstance){
	zval *shared_instance;
	phalcon_fetch_params(0, 1, 0, &shared_instance);
	phalcon_update_property_zval(getThis(), SL("_sharedInstance"), shared_instance);
}

/**
 * Set the service definition
 *
 * @param mixed $definition
 */
PHP_METHOD(Phalcon_DI_Service, setDefinition)
{
	zval *definition;
	phalcon_fetch_params(0, 1, 0, &definition);
	phalcon_update_property_zval(getThis(), SL("_definition"), definition);
}

/**
 * Returns the service definition
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_DI_Service, getDefinition)
{
	RETURN_MEMBER(getThis(), "_definition");
}

/**
 * Resolves the service
 *
 * @param array $parameters
 * @param Phalcon\DiInterface $dependencyInjector
 * @return object
 */
PHP_METHOD(Phalcon_DI_Service, resolve){

	zval *parameters = NULL, *dependency_injector = NULL, name = {}, shared = {}, shared_instance = {}, definition = {}, builder = {};
	int found = 0, ishared = 0;

	phalcon_fetch_params(0, 0, 2, &parameters, &dependency_injector);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!dependency_injector) {
		dependency_injector = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&shared, getThis(), SL("_shared"));
	phalcon_return_property(&shared_instance, getThis(), SL("_sharedInstance"));

	ishared = zend_is_true(&shared);

	/* Check if the service is shared */
	if (ishared && Z_TYPE(shared_instance) != IS_NULL) {
		RETURN_CTORW(&shared_instance);
	}

	phalcon_return_property(&definition, getThis(), SL("_definition"));

	if (Z_TYPE(definition) == IS_STRING) {
		/* String definitions can be class names without implicit parameters */
		if (phalcon_class_exists(&definition, 1) != NULL) {
			found = 1;
			if (Z_TYPE_P(parameters) == IS_ARRAY) {
				RETURN_ON_FAILURE(phalcon_create_instance_params(return_value, &definition, parameters));
			} else {
				RETURN_ON_FAILURE(phalcon_create_instance(return_value, &definition));
			}
		}
	} else if (likely(Z_TYPE(definition) == IS_OBJECT)) {
		/* Object definitions can be a Closure or an already resolved instance */
		found = 1;
		if (instanceof_function_ex(Z_OBJCE(definition), zend_ce_closure, 0)) {
			if (Z_TYPE_P(parameters) == IS_ARRAY) {
				PHALCON_CALL_USER_FUNC_ARRAYW(return_value, &definition, parameters);
			} else {
				PHALCON_CALL_USER_FUNCW(return_value, &definition);
			}
		} else {
			PHALCON_CPY_WRT(return_value, &definition);
		}
	} else if (Z_TYPE(definition) == IS_ARRAY) {
		found = 1;
		/* Array definitions require a 'className' parameter */
		object_init_ex(&builder, phalcon_di_service_builder_ce);

		PHALCON_CALL_METHODW(return_value, &builder, "build", dependency_injector, &definition, parameters);
	}

	if (!EG(exception)) {
		if (found) {
			if (ishared) {
				phalcon_update_property_zval(getThis(), SL("_sharedInstance"), return_value);
			}
			/* Update the shared instance if the service is shared */
			phalcon_update_property_bool(getThis(), SL("_resolved"), 1);
		} else {
			phalcon_return_property(&name, getThis(), SL("_name"));
			PHALCON_THROW_EXCEPTION_FORMATW(phalcon_di_exception_ce, "Service '%s' cannot be resolved", Z_STRVAL(name));
		}
	}
}

/**
 * Changes a parameter in the definition without resolve the service
 *
 * @param long $position
 * @param array $parameter
 * @return Phalcon\DI\Service
 */
PHP_METHOD(Phalcon_DI_Service, setParameter){

	zval *position, *parameter, definition = {}, arguments = {};

	phalcon_fetch_params(0, 2, 0, &position, &parameter);
	PHALCON_ENSURE_IS_LONG(position);

	phalcon_return_property(&definition, getThis(), SL("_definition"));

	if (unlikely(Z_TYPE(definition) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_di_exception_ce, "Definition must be an array to update its parameters");
		return;
	}

	if (unlikely(Z_TYPE_P(parameter) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_di_exception_ce, "The parameter must be an array");
		return;
	}

	/* Update the parameter */
	if (phalcon_array_isset_fetch_str(&arguments, &definition, SL("arguments"))) {
		phalcon_array_update_zval(&arguments, position, parameter, PH_COPY);
	} else {
		array_init_size(&arguments, 1);
		phalcon_array_update_zval(&arguments, position, parameter, PH_COPY);
	}

	phalcon_array_update_str(&definition, SL("arguments"), &arguments, PH_COPY);

	phalcon_update_property_zval(getThis(), SL("_definition"), &definition);

	RETURN_THISW();
}

/**
 * Returns a parameter in a specific position
 *
 * @param int $position
 * @return array
 */
PHP_METHOD(Phalcon_DI_Service, getParameter){

	zval *position, definition = {}, arguments = {};

	phalcon_fetch_params(0, 1, 0, &position);
	PHALCON_ENSURE_IS_LONG(position);

	phalcon_return_property(&definition, getThis(), SL("_definition"));
	if (Z_TYPE(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_di_exception_ce, "Definition must be an array to obtain its parameters");
		return;
	}

	/* Update the parameter */
	if (
		!phalcon_array_isset_fetch_str(&arguments, &definition, SL("arguments")) ||
		!phalcon_array_isset_fetch(return_value, &arguments, position, 0)
	) {
		RETURN_NULL();
	}
}

/**
 * Returns true if the service was resolved
 *
 * @return bool
 */
PHP_METHOD(Phalcon_DI_Service, isResolved)
{
	RETURN_MEMBER(getThis(), "_resolved");
}

/**
 * Restore the internal state of a service
 *
 * @param array $attributes
 * @return Phalcon\DI\Service
 */
PHP_METHOD(Phalcon_DI_Service, __set_state){

	zval *attributes, name = {}, definition = {}, shared = {};

	phalcon_fetch_params(0, 1, 0, &attributes);

	if (
			!phalcon_array_isset_fetch_str(&name, attributes, SL("_name"))
		 || !phalcon_array_isset_fetch_str(&definition, attributes, SL("_definition"))
		 || !phalcon_array_isset_fetch_str(&shared, attributes, SL("_shared"))
	) {
		PHALCON_THROW_EXCEPTION_STRW(spl_ce_BadMethodCallException, "Bad parameters passed to Phalcon\\DI\\Service::__set_state()");
		return;
	}

	object_init_ex(return_value, phalcon_di_service_ce);
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", &name, &definition, &shared);
}
