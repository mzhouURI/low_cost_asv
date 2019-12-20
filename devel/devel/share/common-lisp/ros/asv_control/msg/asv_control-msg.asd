
(cl:in-package :asdf)

(defsystem "asv_control-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "Joy" :depends-on ("_package_Joy"))
    (:file "_package_Joy" :depends-on ("_package"))
  ))