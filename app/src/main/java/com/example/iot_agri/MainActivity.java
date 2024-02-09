package com.example.iot_agri;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import com.google.firebase.database.ChildEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

public class MainActivity extends AppCompatActivity {
    private Spinner pump1Spinner,greaterOrLessSpinnger,pump2Spinnger, greaterOrLessSpinner2;
    private int isPumpSelectedForPH=0,greaterSelected;
    private int isPumpSelectedForTDS=0,greaterSelected2;
    private EditText valueEt,value2Et;
    private TextView pHTv, tempTv,tdsTv, waterTv;

    private DatabaseReference databaseReference= FirebaseDatabase.getInstance().getReference();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        pHTv=findViewById(R.id.phTv);
        tempTv=findViewById(R.id.temperatureTv);
        tdsTv=findViewById(R.id.tdsTv);
        waterTv=findViewById(R.id.waterTv);

        pump1Spinner=findViewById(R.id.spinner);
        pump2Spinnger=findViewById(R.id.spinnerPump2Spn);
        greaterOrLessSpinnger=findViewById(R.id.greateOrLessThanSpn);
        greaterOrLessSpinner2=findViewById(R.id.greateOrLessThan2Spn);
        valueEt=findViewById(R.id.valueEt);
        value2Et=findViewById(R.id.value2Et);
        pump1Spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                isPumpSelectedForPH=position;
                Log.d("SelectedIndex", "Index: " + isPumpSelectedForPH);
            }



            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
                // Handle when nothing is selected
            }
        });

        greaterOrLessSpinnger.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                if(position==1)
                    greaterSelected=0;
                else
                    greaterSelected=1;
                Log.d("SelectedIndex", "Index: " + isPumpSelectedForPH);
            }



            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
                // Handle when nothing is selected
            }
        });





        pump2Spinnger.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                isPumpSelectedForTDS=position;
                Log.d("SelectedIndex", "Index: " + isPumpSelectedForPH);
            }



            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
                // Handle when nothing is selected
            }
        });

        greaterOrLessSpinner2.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                if(position==1)
                    greaterSelected2=0;
                else
                    greaterSelected2=1;
                Log.d("SelectedIndex", "Index: " + isPumpSelectedForPH);
            }



            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
                // Handle when nothing is selected
            }
        });










        databaseReference.child("Device").addChildEventListener(new ChildEventListener() {
            @Override
            public void onChildAdded(@NonNull DataSnapshot snapshot, @Nullable String previousChildName) {

                String[] values = snapshot.getValue().toString().split(":");
                pHTv.setText("pH: "+values[2]);
                tempTv.setText("Temperature: "+values[0]+"°C");
                tdsTv.setText("TDS: "+values[1]+"ppm");
                waterTv.setText("EC: "+values[3]+"mS/cm");

                double[] doubleValues = new double[values.length];


                for (int i = 0; i < values.length; i++) {
                    doubleValues[i] = Double.parseDouble(values[i]);
                }



                String alert="";

                if(doubleValues[0]<15)
                    alert+="Temperature is too low!\n";
                else if(doubleValues[0]>35)
                    alert+="Temperature is too high!\n";
                else
                    alert+="Temperature is normal!\n";

                if(doubleValues[1]<800)
                    alert+="TDS value is too low!\n";
                else if(doubleValues[1]>1500)
                    alert+="TDS value is too high!\n";
                else
                    alert+="TDS is normal!\n";


                if(doubleValues[2]<5)
                    alert+="pH is too low!\n";
                else if(doubleValues[2]>7)
                    alert+="pH is too high!\n";
                else
                    alert+="pH is normal!\n";


                if(doubleValues[3]<1.5)
                    alert+="EC is too low!\n";
                else if(doubleValues[3]>2.0)
                    alert+="EC is too high!\n";
                else
                    alert+="EC is normal!\n";

                ((TextView)findViewById(R.id.noticeTv)).setText(alert);



            }

            @Override
            public void onChildChanged(@NonNull DataSnapshot snapshot, @Nullable String previousChildName) {

            }

            @Override
            public void onChildRemoved(@NonNull DataSnapshot snapshot) {

            }

            @Override
            public void onChildMoved(@NonNull DataSnapshot snapshot, @Nullable String previousChildName) {

            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });



    }

    public void saveValueClick(View view) {

        String value;
        try{
            value=valueEt.getText().toString();
        }catch (Exception e){
            value="0";
        }


        databaseReference.child("Setting").setValue(isPumpSelectedForPH+":"+greaterSelected+":"+value);
    }

    public void save2ValueClick(View view) {


        String value;
        try{
            value=value2Et.getText().toString();
        }catch (Exception e){
            value="0";
        }


        databaseReference.child("Setting2").setValue(isPumpSelectedForTDS+":"+greaterSelected2+":"+value);
    }
}