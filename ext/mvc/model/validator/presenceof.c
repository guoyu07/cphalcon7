
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

#include "mvc/model/validator/presenceof.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\PresenceOf
 *
 * Allows to validate if a filed have a value different of null and empty string ("")
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\PresenceOf;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *  public function validation()
 *  {
 *      $this->validate(new PresenceOf(array(
 *          'field' => 'name',
 *          'message' => 'The name is required'
 *      )));
 *      if ($this->validationHasFailed() == true) {
 *          return false;
 *      }
 *  }
 *
 *}
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_model_validator_presenceof_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_PresenceOf, validate);

static const zend_function_entry phalcon_mvc_model_validator_presenceof_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_PresenceOf, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\PresenceOf initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_PresenceOf){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, PresenceOf, mvc_model_validator_presenceof, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_presenceof_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_presenceof_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_PresenceOf, validate){

	zval *record, option = {}, field_name = {}, value = {}, message = {}, type = {}, is_set_code = {}, code = {};

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field_name, getThis(), "getoption", &option);
	if (Z_TYPE(field_name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	/** 
	 * A value is null when it is identical to null or a empty string
	 */
	PHALCON_CALL_METHODW(&value, record, "readattribute", &field_name);
	if (PHALCON_IS_EMPTY(&value)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			PHALCON_CONCAT_SVS(&message, "'", &field_name, "' is required");
		}

		ZVAL_STRING(&type, "PresenceOf");

		/*
		 * Is code set
		 */
		ZVAL_STRING(&option, ISV(code));

		PHALCON_CALL_METHODW(&is_set_code, getThis(), "issetoption", &option);
		if (zend_is_true(&is_set_code)) {
			PHALCON_CALL_METHODW(&code, getThis(), "getoption", &option);
		} else {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field_name, &type, &code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
