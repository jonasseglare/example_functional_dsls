(ns fdsls.core
  (:require [clojure.string :as cljstr]))

(defn run-dsl [context accumulator body]
  (cond
    (nil? body) accumulator
    (fn? body) (body context accumulator)
    (sequential? body) (reduce (partial run-dsl context) accumulator body)
    :default (ex-info "Invalid body" {:body body})))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;;  Sample DSL for Java source code generation
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; C-c M-p


;;;------- Initial structures -------
(def java-src-context {:visibility :public
                       :static? false
                       :new-line-prefix "\n"})

(def java-src-accumulator "")



(defn visibility-str [ctx]
  (-> ctx :visibility name))

;; (visibility-str (merge java-src-context {:visibility :public}))

(defn static-str [ctx]
  (if (:static? ctx)
    "static" ""))

;; (static-str (merge java-src-context {:static? true}))

(defn output-line [new? & parts]
  (fn [context accumulator]
    (str accumulator
         (if new?
           (:new-line-prefix context)
           "")
         (cljstr/join " " (filter (complement empty?) parts)))))

;; (run-dsl (merge java-src-context {:new-line-prefix "\n  "}) "" (output-line true "Mjao"))

(defn indent-more [& body]
  (fn [context accumulator]
    (run-dsl (update context :new-line-prefix #(str % "  "))
             accumulator body)))

(defn block [& body]
  (fn [context accumulator]
    (run-dsl context
             accumulator
             [(output-line false " {")
              (indent-more body)
              (output-line true "}")])))

(defn named-class [name & body]
  (fn [context accumulator]
    (run-dsl
     context
     accumulator
     [(output-line false (visibility-str context) "class" name)
      (block
       body)])))

(defn static [& body]
  (fn [context accumulator]
    (run-dsl (assoc context :static? true)
             accumulator
             body)))

(defn private [& body]
  (fn [context accumulator]
    (run-dsl (assoc context :visibility :private)
             accumulator
             body)))

(defn variable [type name]
  (fn [context accumulator]
    (run-dsl context
             accumulator
             (output-line
              true
              (visibility-str context)
              (static-str context)
              type name ";"))))





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;;  Samples
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(println (str "\nThe source code is\n"
              (run-dsl java-src-context
                       java-src-accumulator
                       (named-class
                        "Kattskit"
                        (variable "double" "sum")
                        (variable "double" "sumSquares")
                        (variable "int" "count")
                        (private
                         (variable "boolean" "_isDirty"))
                        (static
                         (variable "int" "INSTANCE_COUNTER"))))))
(comment
  (do
    

    (defn with-increment [i & body]
      (fn [context accumulator]
        (run-dsl i accumulator body)))

    (defn increase [context accumulator]
      (+ accumulator context))

    (println "The sum is " (run-dsl 0 0
                                    [increase
                                     increase
                                     (with-increment 10
                                       increase
                                       increase
                                       (with-increment 100
                                         increase
                                         increase)
                                       increase
                                       )]))





    )

  
  )
